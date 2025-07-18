/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbh_core.h"
#include "usb_otg.h"

struct usbh_class_info *usbh_class_info_table_begin = NULL;
struct usbh_class_info *usbh_class_info_table_end = NULL;

usb_slist_t g_bus_head = USB_SLIST_OBJECT_INIT(g_bus_head);

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t ep0_request_buffer[CONFIG_USBHOST_MAX_BUS][CONFIG_USBHOST_REQUEST_BUFFER_LEN];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX struct usb_setup_packet g_setup_buffer[CONFIG_USBHOST_MAX_BUS][CONFIG_USBHOST_MAX_EXTHUBS + 1][CONFIG_USBHOST_MAX_EHPORTS];

/* general descriptor field offsets */
#define DESC_bLength         0 /** Length offset */
#define DESC_bDescriptorType 1 /** Descriptor type offset */

#define USB_DEV_ADDR_MAX         0x7f
#define USB_DEV_ADDR_MARK_OFFSET 5
#define USB_DEV_ADDR_MARK_MASK   0x1f

static int usbh_allocate_devaddr(struct usbh_devaddr_map *devgen)
{
    uint8_t startaddr = devgen->next;
    uint8_t devaddr;
    int index;
    int bitno;

    for (;;) {
        devaddr = devgen->next;
        if (devgen->next >= 0x7f) {
            devgen->next = 2;
        } else {
            devgen->next++;
        }

        index = devaddr >> 5;
        bitno = devaddr & 0x1f;
        if ((devgen->alloctab[index] & (1 << bitno)) == 0) {
            devgen->alloctab[index] |= (1 << bitno);
            return (int)devaddr;
        }

        if (startaddr == devaddr) {
            return -USB_ERR_NOMEM;
        }
    }
}

static int __usbh_free_devaddr(struct usbh_devaddr_map *devgen, uint8_t devaddr)
{
    int index;
    int bitno;

    if ((devaddr > 0) && (devaddr < USB_DEV_ADDR_MAX)) {
        index = devaddr >> USB_DEV_ADDR_MARK_OFFSET;
        bitno = devaddr & USB_DEV_ADDR_MARK_MASK;

        /* Free the address  */
        if ((devgen->alloctab[index] |= (1 << bitno)) != 0) {
            devgen->alloctab[index] &= ~(1 << bitno);
        } else {
            return -1;
        }

        if (devaddr < devgen->next) {
            devgen->next = devaddr;
        }
    }

    return 0;
}

static const struct usbh_class_driver *usbh_find_class_driver(uint8_t class, uint8_t subclass, uint8_t protocol,
                                                              uint16_t vid, uint16_t pid)
{
    struct usbh_class_info *index = NULL;

    for (index = usbh_class_info_table_begin; index < usbh_class_info_table_end; index++) {
        if ((index->match_flags & (USB_CLASS_MATCH_VENDOR | USB_CLASS_MATCH_PRODUCT | USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL)) ==
            (USB_CLASS_MATCH_VENDOR | USB_CLASS_MATCH_PRODUCT | USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL)) {
            if (index->vid == vid && index->pid == pid &&
                index->class == class && index->subclass == subclass && index->protocol == protocol) {
                return index->class_driver;
            }
        } else if ((index->match_flags & (USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL)) ==
                   (USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL)) {
            if (index->class == class && index->subclass == subclass && index->protocol == protocol) {
                return index->class_driver;
            }
        } else if ((index->match_flags & (USB_CLASS_MATCH_VENDOR | USB_CLASS_MATCH_PRODUCT | USB_CLASS_MATCH_INTF_CLASS)) ==
                   (USB_CLASS_MATCH_VENDOR | USB_CLASS_MATCH_PRODUCT | USB_CLASS_MATCH_INTF_CLASS)) {
            if (index->vid == vid && index->pid == pid && index->class == class) {
                return index->class_driver;
            }
        } else if ((index->match_flags & (USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS)) == (USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS)) {
            if (index->class == class && index->subclass == subclass) {
                return index->class_driver;
            }
        } else if ((index->match_flags & (USB_CLASS_MATCH_INTF_CLASS)) == USB_CLASS_MATCH_INTF_CLASS) {
            if (index->class == class) {
                return index->class_driver;
            }
        }
    }
    return NULL;
}

static int parse_device_descriptor(struct usbh_hubport *hport, struct usb_device_descriptor *desc, uint16_t length)
{
    if (desc->bLength != USB_SIZEOF_DEVICE_DESC) {
        USB_LOG_ERR("invalid device bLength 0x%02x\r\n", desc->bLength);
        return -USB_ERR_INVAL;
    } else if (desc->bDescriptorType != USB_DESCRIPTOR_TYPE_DEVICE) {
        USB_LOG_ERR("unexpected device descriptor 0x%02x\r\n", desc->bDescriptorType);
        return -USB_ERR_INVAL;
    } else {
        if (length <= 8) {
            return 0;
        }
#if 0
        USB_LOG_DBG("Device Descriptor:\r\n");
        USB_LOG_DBG("bLength: 0x%02x           \r\n", desc->bLength);
        USB_LOG_DBG("bDescriptorType: 0x%02x   \r\n", desc->bDescriptorType);
        USB_LOG_DBG("bcdUSB: 0x%04x            \r\n", desc->bcdUSB);
        USB_LOG_DBG("bDeviceClass: 0x%02x      \r\n", desc->bDeviceClass);
        USB_LOG_DBG("bDeviceSubClass: 0x%02x   \r\n", desc->bDeviceSubClass);
        USB_LOG_DBG("bDeviceProtocol: 0x%02x   \r\n", desc->bDeviceProtocol);
        USB_LOG_DBG("bMaxPacketSize0: 0x%02x   \r\n", desc->bMaxPacketSize0);
        USB_LOG_DBG("idVendor: 0x%04x          \r\n", desc->idVendor);
        USB_LOG_DBG("idProduct: 0x%04x         \r\n", desc->idProduct);
        USB_LOG_DBG("bcdDevice: 0x%04x         \r\n", desc->bcdDevice);
        USB_LOG_DBG("iManufacturer: 0x%02x     \r\n", desc->iManufacturer);
        USB_LOG_DBG("iProduct: 0x%02x          \r\n", desc->iProduct);
        USB_LOG_DBG("iSerialNumber: 0x%02x     \r\n", desc->iSerialNumber);
        USB_LOG_DBG("bNumConfigurations: 0x%02x\r\n", desc->bNumConfigurations);
#endif
        hport->device_desc.bLength = desc->bLength;
        hport->device_desc.bDescriptorType = desc->bDescriptorType;
        hport->device_desc.bcdUSB = desc->bcdUSB;
        hport->device_desc.bDeviceClass = desc->bDeviceClass;
        hport->device_desc.bDeviceSubClass = desc->bDeviceSubClass;
        hport->device_desc.bDeviceProtocol = desc->bDeviceProtocol;
        hport->device_desc.bMaxPacketSize0 = desc->bMaxPacketSize0;
        hport->device_desc.idVendor = desc->idVendor;
        hport->device_desc.idProduct = desc->idProduct;
        hport->device_desc.bcdDevice = desc->bcdDevice;
        hport->device_desc.iManufacturer = desc->iManufacturer;
        hport->device_desc.iProduct = desc->iProduct;
        hport->device_desc.iSerialNumber = desc->iSerialNumber;
        hport->device_desc.bNumConfigurations = desc->bNumConfigurations;
    }
    return 0;
}

static int parse_config_descriptor(struct usbh_hubport *hport, struct usb_configuration_descriptor *desc, uint16_t length)
{
    struct usb_interface_descriptor *intf_desc;
    struct usb_endpoint_descriptor *ep_desc;
    uint8_t cur_alt_setting = 0xff;
    uint8_t cur_iface = 0xff;
    uint8_t cur_ep = 0xff;
    uint8_t cur_ep_num = 0xff;
    uint32_t desc_len = 0;
    uint8_t *p;

    if (desc->bLength != USB_SIZEOF_CONFIG_DESC) {
        USB_LOG_ERR("invalid config bLength 0x%02x\r\n", desc->bLength);
        return -USB_ERR_INVAL;
    } else if (desc->bDescriptorType != USB_DESCRIPTOR_TYPE_CONFIGURATION) {
        USB_LOG_ERR("unexpected config descriptor 0x%02x\r\n", desc->bDescriptorType);
        return -USB_ERR_INVAL;
    } else {
        if (length <= USB_SIZEOF_CONFIG_DESC) {
            return 0;
        }
#if 0
        USB_LOG_DBG("Config Descriptor:\r\n");
        USB_LOG_DBG("bLength: 0x%02x             \r\n", desc->bLength);
        USB_LOG_DBG("bDescriptorType: 0x%02x     \r\n", desc->bDescriptorType);
        USB_LOG_DBG("wTotalLength: 0x%04x        \r\n", desc->wTotalLength);
        USB_LOG_DBG("bNumInterfaces: 0x%02x      \r\n", desc->bNumInterfaces);
        USB_LOG_DBG("bConfigurationValue: 0x%02x \r\n", desc->bConfigurationValue);
        USB_LOG_DBG("iConfiguration: 0x%02x      \r\n", desc->iConfiguration);
        USB_LOG_DBG("bmAttributes: 0x%02x        \r\n", desc->bmAttributes);
        USB_LOG_DBG("bMaxPower: 0x%02x           \r\n", desc->bMaxPower);
#endif
        hport->config.config_desc.bLength = desc->bLength;
        hport->config.config_desc.bDescriptorType = desc->bDescriptorType;
        hport->config.config_desc.wTotalLength = desc->wTotalLength;
        hport->config.config_desc.bNumInterfaces = desc->bNumInterfaces;
        hport->config.config_desc.bConfigurationValue = desc->bConfigurationValue;
        hport->config.config_desc.iConfiguration = desc->iConfiguration;
        hport->config.config_desc.iConfiguration = desc->iConfiguration;
        hport->config.config_desc.bmAttributes = desc->bmAttributes;
        hport->config.config_desc.bMaxPower = desc->bMaxPower;

        p = (uint8_t *)desc;
        p += USB_SIZEOF_CONFIG_DESC;
        desc_len = USB_SIZEOF_CONFIG_DESC;

        memset(hport->config.intf, 0, sizeof(struct usbh_interface) * CONFIG_USBHOST_MAX_INTERFACES);

        while (p[DESC_bLength] && (desc_len <= length)) {
            switch (p[DESC_bDescriptorType]) {
                case USB_DESCRIPTOR_TYPE_INTERFACE:
                    intf_desc = (struct usb_interface_descriptor *)p;
                    cur_iface = intf_desc->bInterfaceNumber;
                    cur_alt_setting = intf_desc->bAlternateSetting;
                    cur_ep_num = intf_desc->bNumEndpoints;
                    cur_ep = 0;
                    if (cur_iface > (CONFIG_USBHOST_MAX_INTERFACES - 1)) {
                        USB_LOG_ERR("Interface num overflow\r\n");
                        while (1) {
                        }
                    }
                    if (cur_alt_setting > (CONFIG_USBHOST_MAX_INTF_ALTSETTINGS - 1)) {
                        USB_LOG_ERR("Interface altsetting num overflow\r\n");
                        while (1) {
                        }
                    }
                    if (cur_ep_num > CONFIG_USBHOST_MAX_ENDPOINTS) {
                        USB_LOG_ERR("Endpoint num overflow\r\n");
                        while (1) {
                        }
                    }
#if 0
                    USB_LOG_DBG("Interface Descriptor:\r\n");
                    USB_LOG_DBG("bLength: 0x%02x            \r\n", intf_desc->bLength);
                    USB_LOG_DBG("bDescriptorType: 0x%02x    \r\n", intf_desc->bDescriptorType);
                    USB_LOG_DBG("bInterfaceNumber: 0x%02x   \r\n", intf_desc->bInterfaceNumber);
                    USB_LOG_DBG("bAlternateSetting: 0x%02x  \r\n", intf_desc->bAlternateSetting);
                    USB_LOG_DBG("bNumEndpoints: 0x%02x      \r\n", intf_desc->bNumEndpoints);
                    USB_LOG_DBG("bInterfaceClass: 0x%02x    \r\n", intf_desc->bInterfaceClass);
                    USB_LOG_DBG("bInterfaceSubClass: 0x%02x \r\n", intf_desc->bInterfaceSubClass);
                    USB_LOG_DBG("bInterfaceProtocol: 0x%02x \r\n", intf_desc->bInterfaceProtocol);
                    USB_LOG_DBG("iInterface: 0x%02x         \r\n", intf_desc->iInterface);
#endif
                    memcpy(&hport->config.intf[cur_iface].altsetting[cur_alt_setting].intf_desc, intf_desc, 9);
                    hport->config.intf[cur_iface].altsetting_num = cur_alt_setting + 1;
                    break;
                case USB_DESCRIPTOR_TYPE_ENDPOINT:
                    ep_desc = (struct usb_endpoint_descriptor *)p;
                    memcpy(&hport->config.intf[cur_iface].altsetting[cur_alt_setting].ep[cur_ep].ep_desc, ep_desc, 7);
                    cur_ep++;
                    break;

                default:
                    break;
            }
            /* skip to next descriptor */
            p += p[DESC_bLength];
            desc_len += p[DESC_bLength];
        }
    }
    return 0;
}

static void usbh_print_hubport_info(struct usbh_hubport *hport)
{
    USB_LOG_RAW("Device Descriptor:\r\n");
    USB_LOG_RAW("bLength: 0x%02x           \r\n", hport->device_desc.bLength);
    USB_LOG_RAW("bDescriptorType: 0x%02x   \r\n", hport->device_desc.bDescriptorType);
    USB_LOG_RAW("bcdUSB: 0x%04x            \r\n", hport->device_desc.bcdUSB);
    USB_LOG_RAW("bDeviceClass: 0x%02x      \r\n", hport->device_desc.bDeviceClass);
    USB_LOG_RAW("bDeviceSubClass: 0x%02x   \r\n", hport->device_desc.bDeviceSubClass);
    USB_LOG_RAW("bDeviceProtocol: 0x%02x   \r\n", hport->device_desc.bDeviceProtocol);
    USB_LOG_RAW("bMaxPacketSize0: 0x%02x   \r\n", hport->device_desc.bMaxPacketSize0);
    USB_LOG_RAW("idVendor: 0x%04x          \r\n", hport->device_desc.idVendor);
    USB_LOG_RAW("idProduct: 0x%04x         \r\n", hport->device_desc.idProduct);
    USB_LOG_RAW("bcdDevice: 0x%04x         \r\n", hport->device_desc.bcdDevice);
    USB_LOG_RAW("iManufacturer: 0x%02x     \r\n", hport->device_desc.iManufacturer);
    USB_LOG_RAW("iProduct: 0x%02x          \r\n", hport->device_desc.iProduct);
    USB_LOG_RAW("iSerialNumber: 0x%02x     \r\n", hport->device_desc.iSerialNumber);
    USB_LOG_RAW("bNumConfigurations: 0x%02x\r\n", hport->device_desc.bNumConfigurations);

    USB_LOG_RAW("Config Descriptor:\r\n");
    USB_LOG_RAW("bLength: 0x%02x             \r\n", hport->config.config_desc.bLength);
    USB_LOG_RAW("bDescriptorType: 0x%02x     \r\n", hport->config.config_desc.bDescriptorType);
    USB_LOG_RAW("wTotalLength: 0x%04x        \r\n", hport->config.config_desc.wTotalLength);
    USB_LOG_RAW("bNumInterfaces: 0x%02x      \r\n", hport->config.config_desc.bNumInterfaces);
    USB_LOG_RAW("bConfigurationValue: 0x%02x \r\n", hport->config.config_desc.bConfigurationValue);
    USB_LOG_RAW("iConfiguration: 0x%02x      \r\n", hport->config.config_desc.iConfiguration);
    USB_LOG_RAW("bmAttributes: 0x%02x        \r\n", hport->config.config_desc.bmAttributes);
    USB_LOG_RAW("bMaxPower: 0x%02x           \r\n", hport->config.config_desc.bMaxPower);

    for (uint8_t i = 0; i < hport->config.config_desc.bNumInterfaces; i++) {
        for (uint8_t j = 0; j < hport->config.intf[i].altsetting_num; j++) {
            USB_LOG_RAW("\tInterface Descriptor:\r\n");
            USB_LOG_RAW("\tbLength: 0x%02x            \r\n", hport->config.intf[i].altsetting[j].intf_desc.bLength);
            USB_LOG_RAW("\tbDescriptorType: 0x%02x    \r\n", hport->config.intf[i].altsetting[j].intf_desc.bDescriptorType);
            USB_LOG_RAW("\tbInterfaceNumber: 0x%02x   \r\n", hport->config.intf[i].altsetting[j].intf_desc.bInterfaceNumber);
            USB_LOG_RAW("\tbAlternateSetting: 0x%02x  \r\n", hport->config.intf[i].altsetting[j].intf_desc.bAlternateSetting);
            USB_LOG_RAW("\tbNumEndpoints: 0x%02x      \r\n", hport->config.intf[i].altsetting[j].intf_desc.bNumEndpoints);
            USB_LOG_RAW("\tbInterfaceClass: 0x%02x    \r\n", hport->config.intf[i].altsetting[j].intf_desc.bInterfaceClass);
            USB_LOG_RAW("\tbInterfaceSubClass: 0x%02x \r\n", hport->config.intf[i].altsetting[j].intf_desc.bInterfaceSubClass);
            USB_LOG_RAW("\tbInterfaceProtocol: 0x%02x \r\n", hport->config.intf[i].altsetting[j].intf_desc.bInterfaceProtocol);
            USB_LOG_RAW("\tiInterface: 0x%02x         \r\n", hport->config.intf[i].altsetting[j].intf_desc.iInterface);

            for (uint8_t k = 0; k < hport->config.intf[i].altsetting[j].intf_desc.bNumEndpoints; k++) {
                USB_LOG_RAW("\t\tEndpoint Descriptor:\r\n");
                USB_LOG_RAW("\t\tbLength: 0x%02x          \r\n", hport->config.intf[i].altsetting[j].ep[k].ep_desc.bLength);
                USB_LOG_RAW("\t\tbDescriptorType: 0x%02x  \r\n", hport->config.intf[i].altsetting[j].ep[k].ep_desc.bDescriptorType);
                USB_LOG_RAW("\t\tbEndpointAddress: 0x%02x \r\n", hport->config.intf[i].altsetting[j].ep[k].ep_desc.bEndpointAddress);
                USB_LOG_RAW("\t\tbmAttributes: 0x%02x     \r\n", hport->config.intf[i].altsetting[j].ep[k].ep_desc.bmAttributes);
                USB_LOG_RAW("\t\twMaxPacketSize: 0x%04x   \r\n", hport->config.intf[i].altsetting[j].ep[k].ep_desc.wMaxPacketSize);
                USB_LOG_RAW("\t\tbInterval: 0x%02x        \r\n", hport->config.intf[i].altsetting[j].ep[k].ep_desc.bInterval);
            }
        }
    }
}

static int usbh_get_default_mps(int speed)
{
    switch (speed) {
        case USB_SPEED_LOW: /* For low speed, we use 8 bytes */
            return 8;
        case USB_SPEED_FULL: /* For full or high speed, we use 64 bytes */
        case USB_SPEED_HIGH:
            return 64;
        case USB_SPEED_SUPER: /* For super speed , we must use 512 bytes */
        case USB_SPEED_SUPER_PLUS:
            return 512;
        default:
            return 64;
    }
}

int usbh_free_devaddr(struct usbh_hubport *hport)
{
#ifndef CONFIG_USBHOST_XHCI
    if (hport->dev_addr > 0) {
        __usbh_free_devaddr(&hport->bus->devgen, hport->dev_addr);
    }
#endif

    hport->dev_addr = 0;
    return 0;
}

int usbh_get_string_desc(struct usbh_hubport *hport, uint8_t index, uint8_t *output)
{
    struct usb_setup_packet *setup = hport->setup;
    int ret;
    uint8_t *src;
    uint8_t *dst;
    uint16_t len;
    uint16_t i = 2;
    uint16_t j = 0;

    /* Get Manufacturer string */
    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = USB_REQUEST_GET_DESCRIPTOR;
    setup->wValue = (uint16_t)((USB_DESCRIPTOR_TYPE_STRING << 8) | index);
    setup->wIndex = 0x0409;
    setup->wLength = 255;

    ret = usbh_control_transfer(hport, setup, ep0_request_buffer[hport->bus->busid]);
    if (ret < 0) {
        return ret;
    }

    src = ep0_request_buffer[hport->bus->busid];
    dst = output;
    len = src[0];

    while (i < len) {
        dst[j] = src[i];
        i += 2;
        j++;
    }

    return 0;
}

int usbh_set_interface(struct usbh_hubport *hport, uint8_t intf, uint8_t altsetting)
{
    struct usb_setup_packet *setup = hport->setup;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = USB_REQUEST_SET_INTERFACE;
    setup->wValue = altsetting;
    setup->wIndex = intf;
    setup->wLength = 0;

    return usbh_control_transfer(hport, setup, NULL);
}

int usbh_enumerate(struct usbh_hubport *hport)
{
    struct usb_interface_descriptor *intf_desc;
    struct usb_setup_packet *setup;
    struct usb_device_descriptor *dev_desc;
    struct usb_endpoint_descriptor *ep;
    int dev_addr;
    uint16_t ep_mps;
    int ret;

    hport->setup = &g_setup_buffer[hport->bus->busid][hport->parent->index - 1][hport->port - 1];
    setup = hport->setup;
    ep = &hport->ep0;

    /* Config EP0 mps from speed */
    ep->bEndpointAddress = 0x00;
    ep->bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT;
    ep->bmAttributes = USB_ENDPOINT_TYPE_CONTROL;
    ep->wMaxPacketSize = usbh_get_default_mps(hport->speed);
    ep->bInterval = 0;
    ep->bLength = 7;

    /* Configure EP0 with zero address */
    hport->dev_addr = 0;

    /* Read the first 8 bytes of the device descriptor */
    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = USB_REQUEST_GET_DESCRIPTOR;
    setup->wValue = (uint16_t)((USB_DESCRIPTOR_TYPE_DEVICE << 8) | 0);
    setup->wIndex = 0;
    setup->wLength = 8;

    ret = usbh_control_transfer(hport, setup, ep0_request_buffer[hport->bus->busid]);
    if (ret < 0) {
        USB_LOG_ERR("Failed to get device descriptor,errorcode:%d\r\n", ret);
        goto errout;
    }

    parse_device_descriptor(hport, (struct usb_device_descriptor *)ep0_request_buffer[hport->bus->busid], 8);

    /* Extract the correct max packetsize from the device descriptor */
    dev_desc = (struct usb_device_descriptor *)ep0_request_buffer[hport->bus->busid];
    if (dev_desc->bcdUSB >= USB_3_0) {
        ep_mps = 1 << dev_desc->bMaxPacketSize0;
    } else {
        ep_mps = dev_desc->bMaxPacketSize0;
    }

    USB_LOG_DBG("Device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\r\n",
                dev_desc->bcdUSB, dev_desc->bDeviceClass, dev_desc->bDeviceSubClass,
                dev_desc->bDeviceProtocol, ep_mps);

    /* Reconfigure EP0 with the correct maximum packet size */
    ep->wMaxPacketSize = ep_mps;

#ifdef CONFIG_USBHOST_XHCI
    extern int usbh_get_xhci_devaddr(usbh_pipe_t * pipe);

    /* Assign a function address to the device connected to this port */
    dev_addr = usbh_get_xhci_devaddr(hport->ep0);
    if (dev_addr < 0) {
        USB_LOG_ERR("Failed to allocate devaddr,errorcode:%d\r\n", ret);
        goto errout;
    }
#else
    /* Assign a function address to the device connected to this port */
    dev_addr = usbh_allocate_devaddr(&hport->bus->devgen);
    if (dev_addr < 0) {
        USB_LOG_ERR("Failed to allocate devaddr,errorcode:%d\r\n", ret);
        goto errout;
    }
#endif

    /* Set the USB device address */
    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = USB_REQUEST_SET_ADDRESS;
    setup->wValue = dev_addr;
    setup->wIndex = 0;
    setup->wLength = 0;

    ret = usbh_control_transfer(hport, setup, NULL);
    if (ret < 0) {
        USB_LOG_ERR("Failed to set devaddr,errorcode:%d\r\n", ret);
        goto errout;
    }

    /* Wait device set address completely */
    usb_osal_msleep(2);

    /*Reconfigure EP0 with the correct address */
    hport->dev_addr = dev_addr;

    /* Read the full device descriptor */
    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = USB_REQUEST_GET_DESCRIPTOR;
    setup->wValue = (uint16_t)((USB_DESCRIPTOR_TYPE_DEVICE << 8) | 0);
    setup->wIndex = 0;
    setup->wLength = USB_SIZEOF_DEVICE_DESC;

    ret = usbh_control_transfer(hport, setup, ep0_request_buffer[hport->bus->busid]);
    if (ret < 0) {
        USB_LOG_ERR("Failed to get full device descriptor,errorcode:%d\r\n", ret);
        goto errout;
    }

    parse_device_descriptor(hport, (struct usb_device_descriptor *)ep0_request_buffer[hport->bus->busid], USB_SIZEOF_DEVICE_DESC);
    USB_LOG_INFO("New device found,idVendor:%04x,idProduct:%04x,bcdDevice:%04x\r\n",
                 ((struct usb_device_descriptor *)ep0_request_buffer[hport->bus->busid])->idVendor,
                 ((struct usb_device_descriptor *)ep0_request_buffer[hport->bus->busid])->idProduct,
                 ((struct usb_device_descriptor *)ep0_request_buffer[hport->bus->busid])->bcdDevice);

    /* Read the first 9 bytes of the config descriptor */
    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = USB_REQUEST_GET_DESCRIPTOR;
    setup->wValue = (uint16_t)((USB_DESCRIPTOR_TYPE_CONFIGURATION << 8) | 0);
    setup->wIndex = 0;
    setup->wLength = USB_SIZEOF_CONFIG_DESC;

    ret = usbh_control_transfer(hport, setup, ep0_request_buffer[hport->bus->busid]);
    if (ret < 0) {
        USB_LOG_ERR("Failed to get config descriptor,errorcode:%d\r\n", ret);
        goto errout;
    }

    parse_config_descriptor(hport, (struct usb_configuration_descriptor *)ep0_request_buffer[hport->bus->busid], USB_SIZEOF_CONFIG_DESC);

    /* Read the full size of the configuration data */
    uint16_t wTotalLength = ((struct usb_configuration_descriptor *)ep0_request_buffer[hport->bus->busid])->wTotalLength;

    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = USB_REQUEST_GET_DESCRIPTOR;
    setup->wValue = (uint16_t)((USB_DESCRIPTOR_TYPE_CONFIGURATION << 8) | 0);
    setup->wIndex = 0;
    setup->wLength = wTotalLength;

    ret = usbh_control_transfer(hport, setup, ep0_request_buffer[hport->bus->busid]);
    if (ret < 0) {
        USB_LOG_ERR("Failed to get full config descriptor,errorcode:%d\r\n", ret);
        goto errout;
    }

    ret = parse_config_descriptor(hport, (struct usb_configuration_descriptor *)ep0_request_buffer[hport->bus->busid], wTotalLength);
    if (ret < 0) {
        USB_LOG_ERR("Parse config fail\r\n");
        goto errout;
    }
    USB_LOG_INFO("The device has %d interfaces\r\n", ((struct usb_configuration_descriptor *)ep0_request_buffer[hport->bus->busid])->bNumInterfaces);
    hport->raw_config_desc = usb_malloc(wTotalLength);
    hport->raw_config_desc_len = wTotalLength;
    if (hport->raw_config_desc == NULL) {
        ret = -USB_ERR_NOMEM;
        USB_LOG_ERR("No memory to alloc for raw_config_desc\r\n");
        goto errout;
    }
    memcpy(hport->raw_config_desc, ep0_request_buffer[hport->bus->busid], wTotalLength);
#ifdef CONFIG_USBHOST_GET_STRING_DESC
    uint8_t string_buffer[128];

    /* Get Manufacturer string */
    memset(string_buffer, 0, 128);
    ret = usbh_get_string_desc(hport, USB_STRING_MFC_INDEX, string_buffer);
    if (ret < 0) {
        USB_LOG_ERR("Failed to get Manufacturer string,errorcode:%d\r\n", ret);
        goto errout;
    }

    USB_LOG_INFO("Manufacturer: %s\r\n", string_buffer);

    /* Get Product string */
    memset(string_buffer, 0, 128);
    ret = usbh_get_string_desc(hport, USB_STRING_PRODUCT_INDEX, string_buffer);
    if (ret < 0) {
        USB_LOG_ERR("Failed to get get Product string,errorcode:%d\r\n", ret);
        goto errout;
    }

    USB_LOG_INFO("Product: %s\r\n", string_buffer);

    /* Get SerialNumber string */
    memset(string_buffer, 0, 128);
    ret = usbh_get_string_desc(hport, USB_STRING_SERIAL_INDEX, string_buffer);
    if (ret < 0) {
        USB_LOG_ERR("Failed to get get SerialNumber string,errorcode:%d\r\n", ret);
        goto errout;
    }

    USB_LOG_INFO("SerialNumber: %s\r\n", string_buffer);
#endif
    /* Select device configuration 1 */
    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = USB_REQUEST_SET_CONFIGURATION;
    setup->wValue = 1;
    setup->wIndex = 0;
    setup->wLength = 0;

    ret = usbh_control_transfer(hport, setup, NULL);
    if (ret < 0) {
        USB_LOG_ERR("Failed to set configuration,errorcode:%d\r\n", ret);
        goto errout;
    }

#ifdef CONFIG_USBHOST_MSOS_ENABLE
    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_VENDOR | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = CONFIG_USBHOST_MSOS_VENDOR_CODE;
    setup->wValue = 0;
    setup->wIndex = 0x0004;
    setup->wLength = 16;

    ret = usbh_control_transfer(hport, setup, ep0_request_buffer[hport->bus->busid]);
    if (ret < 0 && (ret != -EPERM)) {
        USB_LOG_ERR("Failed to get msosv1 compat id,errorcode:%d\r\n", ret);
        goto errout;
    }
#endif
    USB_LOG_INFO("Enumeration success, start loading class driver\r\n");
    /*search supported class driver*/
    for (uint8_t i = 0; i < hport->config.config_desc.bNumInterfaces; i++) {
        intf_desc = &hport->config.intf[i].altsetting[0].intf_desc;

        struct usbh_class_driver *class_driver = (struct usbh_class_driver *)usbh_find_class_driver(intf_desc->bInterfaceClass, intf_desc->bInterfaceSubClass, intf_desc->bInterfaceProtocol, hport->device_desc.idVendor, hport->device_desc.idProduct);

        if (class_driver == NULL) {
            USB_LOG_ERR("do not support Class:0x%02x,Subclass:0x%02x,Protocl:0x%02x\r\n",
                        intf_desc->bInterfaceClass,
                        intf_desc->bInterfaceSubClass,
                        intf_desc->bInterfaceProtocol);

            continue;
        }
        hport->config.intf[i].class_driver = class_driver;
        USB_LOG_INFO("Loading %s class driver (interfaces :%d)\r\n", class_driver->driver_name, i);
        usb_osal_mutex_take(hport->mutex);
        ret = CLASS_CONNECT(hport, i);
        usb_osal_mutex_give(hport->mutex);
        if (ret < 0)
            USB_LOG_WRN("%s Class connect abnormal :%d)\r\n", __func__, ret);
    }

    if (hport->event)
        usb_osal_event_send(hport->event, USBH_EVENT_ENUMERATE);
errout:
    if (hport->raw_config_desc) {
        usb_free(hport->raw_config_desc);
        hport->raw_config_desc = NULL;
        hport->raw_config_desc_len = 0;
    }
    return ret;
}

int usbh_hubport_enumerate_wait(struct usbh_hubport *hport)
{
    int ret = 0;
    uint32_t cur_event = 0;

    if (hport->event == NULL)
        return -USB_ERR_NOTSUPP;

    ret = usb_osal_event_recv(hport->event, USBH_EVENT_ENUMERATE,
                                USB_OSAL_EVENT_FLAG_OR, USB_OSAL_WAITING_FOREVER,
                                &cur_event);

    USB_LOG_DBG("%s cur_event: %d \n", __func__, cur_event);

    return ret;
}

struct usbh_bus *usbh_alloc_bus(uint8_t busid, uint32_t reg_base)
{
    struct usbh_bus *bus;
    struct usbh_hub *hub;

    if (busid > CONFIG_USBHOST_MAX_BUS) {
        USB_LOG_ERR("bus overflow\r\n");
        while (1) {
        }
    }

    bus = usb_malloc(sizeof(struct usbh_bus));
    if (bus == NULL) {
        USB_LOG_ERR("No memory to alloc bus\r\n");
        while (1) {
        }
    }

    memset(bus, 0, sizeof(struct usbh_bus));
    bus->busid = busid;
    bus->hcd.hcd_id = busid;
    bus->hcd.reg_base = reg_base;

    /* devaddr 1 is for roothub */
    bus->devgen.next = 2;

    usb_slist_init(&bus->hub_list);

    hub = &bus->hcd.roothub;
    hub->connected = true;
    hub->index = 1;
    hub->is_roothub = true;
    hub->parent = NULL;
    hub->hub_addr = 1;
    hub->hub_desc.bNbrPorts = CONFIG_USBHOST_MAX_RHPORTS;
    hub->int_buffer = bus->hcd.roothub_intbuf;
    hub->bus = bus;

    usb_slist_init(&bus->hub_list);
    usb_slist_add_tail(&bus->hub_list, &hub->list);
    usb_slist_add_tail(&g_bus_head, &bus->list);

    return bus;
}

int usbh_initialize(struct usbh_bus *bus)
{
#ifdef __ARMCC_VERSION /* ARM C Compiler */
    extern const int usbh_class_info$$Base;
    extern const int usbh_class_info$$Limit;
    usbh_class_info_table_begin = (struct usbh_class_info *)&usbh_class_info$$Base;
    usbh_class_info_table_end = (struct usbh_class_info *)&usbh_class_info$$Limit;
#elif defined(__GNUC__)
    extern uint32_t __usbh_class_info_start__;
    extern uint32_t __usbh_class_info_end__;
    usbh_class_info_table_begin = (struct usbh_class_info *)&__usbh_class_info_start__;
    usbh_class_info_table_end = (struct usbh_class_info *)&__usbh_class_info_end__;
#elif defined(__ICCARM__) || defined(__ICCRX__) || defined(__ICCRISCV__)
    usbh_class_info_table_begin = (struct usbh_class_info *)__section_begin("usbh_class_info");
    usbh_class_info_table_end = (struct usbh_class_info *)__section_end("usbh_class_info");
#endif
    return usbh_hub_initialize(bus);
}

int usbh_deinitialize(struct usbh_bus *bus)
{
    usbh_hub_deinitialize(bus);

    usb_slist_init(&bus->hub_list);
    usb_slist_remove(&g_bus_head, &bus->list);

    return 0;
}

int usbh_control_transfer(struct usbh_hubport *hport, struct usb_setup_packet *setup, uint8_t *buffer)
{
    struct usbh_urb *urb;
    int ret;

    urb = &hport->ep0_urb;

    usb_osal_mutex_take(hport->mutex);

    memset(urb, 0, sizeof(struct usbh_urb));
    usbh_control_urb_fill(urb, hport, setup, buffer, setup->wLength, CONFIG_USBHOST_CONTROL_TRANSFER_TIMEOUT, NULL, NULL);
    ret = usbh_submit_urb(urb);
    if (ret == 0) {
        ret = urb->actual_length;
    }

    usb_osal_mutex_give(hport->mutex);
    return ret;
}

void *usbh_find_class_instance(const char *devname)
{
    struct usbh_hubport *hport;
    usb_slist_t *hub_list;
    usb_slist_t *bus_list;

    usb_slist_for_each(bus_list, &g_bus_head)
    {
        struct usbh_bus *bus = usb_slist_entry(bus_list, struct usbh_bus, list);
        usb_slist_for_each(hub_list, &bus->hub_list)
        {
            struct usbh_hub *hub = usb_slist_entry(hub_list, struct usbh_hub, list);
            for (uint8_t port = 0; port < hub->hub_desc.bNbrPorts; port++) {
                hport = &hub->child[port];
                if (hport->connected) {
                    for (uint8_t itf = 0; itf < hport->config.config_desc.bNumInterfaces; itf++) {
                        if ((strncmp(hport->config.intf[itf].devname, devname, CONFIG_USBHOST_DEV_NAMELEN) == 0) && hport->config.intf[itf].priv)
                            return hport->config.intf[itf].priv;
                    }
                }
            }
        }
    }
    return NULL;
}

int lsusb(int argc, char **argv)
{
    usb_slist_t *hub_list;
    usb_slist_t *bus_list;
    struct usbh_hubport *hport;

    if (argc < 2) {
        USB_LOG_RAW("Usage: lsusb [options]...\r\n");
        USB_LOG_RAW("List USB devices\r\n");
        USB_LOG_RAW("  -v, --verbose\r\n");
        USB_LOG_RAW("      Increase verbosity (show descriptors)\r\n");
        // USB_LOG_RAW("  -s [[bus]:[devnum]]\r\n");
        // USB_LOG_RAW("      Show only devices with specified device and/or bus numbers (in decimal)\r\n");
        // USB_LOG_RAW("  -d vendor:[product]\r\n");
        // USB_LOG_RAW("      Show only devices with the specified vendor and product ID numbers (in hexadecimal)\r\n");
        USB_LOG_RAW("  -t, --tree\r\n");
        USB_LOG_RAW("      Dump the physical USB device hierachy as a tree\r\n");
        USB_LOG_RAW("  -V, --version\r\n");
        USB_LOG_RAW("      Show version of program\r\n");
        USB_LOG_RAW("  -h, --help\r\n");
        USB_LOG_RAW("      Show usage and help\r\n");
        return 0;
    }
    if (argc > 3) {
        return 0;
    }

    if (strcmp(argv[1], "-t") == 0) {
        usb_slist_for_each(bus_list, &g_bus_head)
        {
            struct usbh_bus *bus = usb_slist_entry(bus_list, struct usbh_bus, list);
            usb_slist_for_each(hub_list, &bus->hub_list)
            {
                struct usbh_hub *hub = usb_slist_entry(hub_list, struct usbh_hub, list);

                if (hub->is_roothub) {
                    USB_LOG_RAW("/: Bus %u, Hub %u, ports=%u, is roothub\r\n",
                                bus->busid,
                                hub->index,
                                hub->hub_desc.bNbrPorts);
                } else {
                    USB_LOG_RAW("/: Bus %u, Hub %u, ports=%u, mounted on Hub %02u:Port %u\r\n",
                                bus->busid,
                                hub->index,
                                hub->hub_desc.bNbrPorts,
                                hub->parent->parent->index,
                                hub->parent->port);
                }

                for (uint8_t port = 0; port < hub->hub_desc.bNbrPorts; port++) {
                    hport = &hub->child[port];
                    if (hport->connected) {
                        for (uint8_t i = 0; i < hport->config.config_desc.bNumInterfaces; i++) {
                            if (hport->config.intf[i].class_driver->driver_name) {
                                USB_LOG_RAW("\t|__Port %u, dev addr:0x%02x, If %u, ClassDriver=%s\r\n",
                                            hport->port,
                                            hport->dev_addr,
                                            i,
                                            hport->config.intf[i].class_driver->driver_name);
                            }
                        }
                    }
                }
            }
        }
    }

    if (strcmp(argv[1], "-v") == 0) {
        usb_slist_for_each(bus_list, &g_bus_head)
        {
            struct usbh_bus *bus = usb_slist_entry(bus_list, struct usbh_bus, list);
            usb_slist_for_each(hub_list, &bus->hub_list)
            {
                struct usbh_hub *hub = usb_slist_entry(hub_list, struct usbh_hub, list);
                for (uint8_t port = 0; port < hub->hub_desc.bNbrPorts; port++) {
                    hport = &hub->child[port];
                    if (hport->connected) {
                        USB_LOG_RAW("Bus %u, Hub %02u, Port %u, dev addr:0x%02x, VID:PID 0x%04x:0x%04x\r\n",
                                    bus->busid,
                                    hub->index,
                                    hport->port,
                                    hport->dev_addr,
                                    hport->device_desc.idVendor,
                                    hport->device_desc.idProduct);
                        usbh_print_hubport_info(hport);
                    }
                }
            }
        }
    }

    return 0;
}

#if defined(AIC_USING_USB0_HOST) || defined(AIC_USING_USB0_OTG)
struct usbh_bus *usb_ehci0_hs_bus;
#endif
#ifdef AIC_USING_USB1_HOST
struct usbh_bus *usb_ehci1_hs_bus;
#endif

int usbh_init(void)
{
#if defined(AIC_USING_USB0_HOST) || defined(AIC_USING_USB0_OTG) || defined(AIC_USING_USB1_HOST)
    int bus_id = 0;
    int ret = 0;
#endif

#if defined(AIC_USING_USB0_HOST) || defined(AIC_USING_USB0_OTG)
    usb_ehci0_hs_bus = usbh_alloc_bus(bus_id, USB_HOST0_BASE);
    #if defined(AIC_USING_USB0_OTG) && !defined(AIC_BOOTLOADER)
    ret = usb_otg_register_host(0, usb_ehci0_hs_bus);
    #endif
    if (!ret)
        ret = usbh_initialize(usb_ehci0_hs_bus);
    bus_id++;
#endif

#ifdef AIC_USING_USB1_HOST
    usb_ehci1_hs_bus = usbh_alloc_bus(bus_id, USB_HOST1_BASE);
    ret = usbh_initialize(usb_ehci1_hs_bus);
    bus_id++;
#endif

    return ret;
}

#if defined(KERNEL_RTTHREAD)
#include <rtthread.h>
#include <rtdevice.h>

INIT_ENV_EXPORT(usbh_init);

#if defined (RT_USING_FINSH)
#include <finsh.h>

MSH_CMD_EXPORT_ALIAS(lsusb, lsusb, list usb device);
#endif
#endif
