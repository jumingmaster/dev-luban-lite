/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *                                                                            
 *  Version:    C, Version 2.1
 *                                                                            
 *  File:       dhry.h (part 1 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *                      Siemens AG, E STE 35
 *                      Postfach 3240
 *                      8520 Erlangen
 *                      Germany (West)
 *                              Phone:  [xxx-49]-9131-7-20330
 *                                      (8-17 Central European Time)
 *                              Usenet: ..!mcvax!unido!estevax!weicker
 *
 *              Original Version (in Ada) published in
 *              "Communications of the ACM" vol. 27., no. 10 (Oct. 1984),
 *              pp. 1013 - 1030, together with the statistics
 *              on which the distribution of statements etc. is based.
 *
 *              In this C version, the following C library functions are used:
 *              - strcpy, strcmp (inside the measurement loop)
 *              - printf, scanf (outside the measurement loop)
 *              In addition, Berkeley UNIX system calls "times ()" or "time ()"
 *              are used for execution time measurement. For measurements
 *              on other systems, these calls have to be changed.
 *
 *  Collection of Results:
 *              Reinhold Weicker (address see above) and
 *              
 *              Rick Richardson
 *              PC Research. Inc.
 *              94 Apple Orchard Drive
 *              Tinton Falls, NJ 07724
 *                      Phone:  (201) 389-8963 (9-17 EST)               
 *                      Usenet: ...!uunet!pcrat!rick
 *
 *      Please send results to Rick Richardson and/or Reinhold Weicker.
 *      Complete information should be given on hardware and software used.
 *      Hardware information includes: Machine type, CPU, type and size
 *      of caches; for microprocessors: clock frequency, memory speed
 *      (number of wait states).
 *      Software information includes: Compiler (and runtime library)
 *      manufacturer and version, compilation switches, OS version.
 *      The Operating System version may give an indication about the
 *      compiler; Dhrystone itself performs no OS calls in the measurement loop.
 *
 *      The complete output generated by the program should be mailed
 *      such that at least some checks for correctness can be made.
 *
 ***************************************************************************
 *
 *  History:    This version C/2.1 has been made for two reasons:
 *
 *              1) There is an obvious need for a common C version of
 *              Dhrystone, since C is at present the most popular system
 *              programming language for the class of processors
 *              (microcomputers, minicomputers) where Dhrystone is used most.
 *              There should be, as far as possible, only one C version of
 *              Dhrystone such that results can be compared without
 *              restrictions. In the past, the C versions distributed
 *              by Rick Richardson (Version 1.1) and by Reinhold Weicker
 *              had small (though not significant) differences.
 *
 *              2) As far as it is possible without changes to the Dhrystone
 *              statistics, optimizing compilers should be prevented from
 *              removing significant statements.
 *
 *              This C version has been developed in cooperation with
 *              Rick Richardson (Tinton Falls, NJ), it incorporates many
 *              ideas from the "Version 1.1" distributed previously by
 *              him over the UNIX network Usenet.
 *              I also thank Chaim Benedelac (National Semiconductor),
 *              David Ditzel (SUN), Earl Killian and John Mashey (MIPS),
 *              Alan Smith and Rafael Saavedra-Barrera (UC at Berkeley)
 *              for their help with comments on earlier versions of the
 *              benchmark.
 *
 *  Changes:    In the initialization part, this version follows mostly
 *              Rick Richardson's version distributed via Usenet, not the
 *              version distributed earlier via floppy disk by Reinhold Weicker.
 *              As a concession to older compilers, names have been made
 *              unique within the first 8 characters.
 *              Inside the measurement loop, this version follows the
 *              version previously distributed by Reinhold Weicker.
 *
 *              At several places in the benchmark, code has been added,
 *              but within the measurement loop only in branches that 
 *              are not executed. The intention is that optimizing compilers
 *              should be prevented from moving code out of the measurement
 *              loop, or from removing code altogether. Since the statements
 *              that are executed within the measurement loop have NOT been
 *              changed, the numbers defining the "Dhrystone distribution"
 *              (distribution of statements, operand types and locality)
 *              still hold. Except for sophisticated optimizing compilers,
 *              execution times for this version should be the same as
 *              for previous versions.
 *              
 *              Since it has proven difficult to subtract the time for the
 *              measurement loop overhead in a correct way, the loop check
 *              has been made a part of the benchmark. This does have
 *              an impact - though a very minor one - on the distribution
 *              statistics which have been updated for this version.
 *
 *              All changes within the measurement loop are described
 *              and discussed in the companion paper "Rationale for
 *              Dhrystone version 2".
 *
 *              Because of the self-imposed limitation that the order and
 *              distribution of the executed statements should not be
 *              changed, there are still cases where optimizing compilers
 *              may not generate code for some statements. To a certain
 *              degree, this is unavoidable for small synthetic benchmarks.
 *              Users of the benchmark are advised to check code listings
 *              whether code is generated for all statements of Dhrystone.
 *
 *              Version 2.1 is identical to version 2.0 distributed via
 *              the UNIX network Usenet in March 1988 except that it corrects
 *              some minor deficiencies that were found by users of version 2.0.
 *              The only change within the measurement loop is that a
 *              non-executed "else" part was added to the "if" statement in
 *              Func_3, and a non-executed "else" part removed from Proc_3.
 *
 ***************************************************************************
 *
 * Defines:     The following "Defines" are possible:
 *              -DREG=register          (default: Not defined)
 *                      As an approximation to what an average C programmer
 *                      might do, the "register" storage class is applied
 *                      (if enabled by -DREG=register)
 *                      - for local variables, if they are used (dynamically)
 *                        five or more times
 *                      - for parameters if they are used (dynamically)
 *                        six or more times
 *                      Note that an optimal "register" strategy is
 *                      compiler-dependent, and that "register" declarations
 *                      do not necessarily lead to faster execution.
 *              -DNOSTRUCTASSIGN        (default: Not defined)
 *                      Define if the C compiler does not support
 *                      assignment of structures.
 *              -DNOENUMS               (default: Not defined)
 *                      Define if the C compiler does not support
 *                      enumeration types.
 *              -DTIMES                 (default)
 *              -DTIME
 *                      The "times" function of UNIX (returning process times)
 *                      or the "time" function (returning wallclock time)
 *                      is used for measurement. 
 *                      For single user machines, "time ()" is adequate. For
 *                      multi-user machines where you cannot get single-user
 *                      access, use the "times ()" function. If you have
 *                      neither, use a stopwatch in the dead of night.
 *                      "printf"s are provided marking the points "Start Timer"
 *                      and "Stop Timer". DO NOT use the UNIX "time(1)"
 *                      command, as this will measure the total time to
 *                      run this program, which will (erroneously) include
 *                      the time to allocate storage (malloc) and to perform
 *                      the initialization.
 *              -DHZ=nnn
 *                      In Berkeley UNIX, the function "times" returns process
 *                      time in 1/HZ seconds, with HZ = 60 for most systems.
 *                      CHECK YOUR SYSTEM DESCRIPTION BEFORE YOU JUST APPLY
 *                      A VALUE.
 *
 ***************************************************************************
 *
 *  Compilation model and measurement (IMPORTANT):
 *
 *  This C version of Dhrystone consists of three files:
 *  - dhry.h (this file, containing global definitions and comments)
 *  - dhry_1.c (containing the code corresponding to Ada package Pack_1)
 *  - dhry_2.c (containing the code corresponding to Ada package Pack_2)
 *
 *  The following "ground rules" apply for measurements:
 *  - Separate compilation
 *  - No procedure merging
 *  - Otherwise, compiler optimizations are allowed but should be indicated
 *  - Default results are those without register declarations
 *  See the companion paper "Rationale for Dhrystone Version 2" for a more
 *  detailed discussion of these ground rules.
 *
 *  For 16-Bit processors (e.g. 80186, 80286), times for all compilation
 *  models ("small", "medium", "large" etc.) should be given if possible,
 *  together with a definition of these models for the compiler system used.
 *
 **************************************************************************
 *
 *  Dhrystone (C version) statistics:
 *
 *  [Comment from the first distribution, updated for version 2.
 *   Note that because of language differences, the numbers are slightly
 *   different from the Ada version.]
 *
 *  The following program contains statements of a high level programming
 *  language (here: C) in a distribution considered representative:           
 *
 *    assignments                  52 (51.0 %)
 *    control statements           33 (32.4 %)
 *    procedure, function calls    17 (16.7 %)
 *
 *  103 statements are dynamically executed. The program is balanced with
 *  respect to the three aspects:                                             
 *
 *    - statement type
 *    - operand type
 *    - operand locality
 *         operand global, local, parameter, or constant.                     
 *
 *  The combination of these three aspects is balanced only approximately.    
 *
 *  1. Statement Type:                                                        
 *  -----------------             number
 *
 *     V1 = V2                     9
 *       (incl. V1 = F(..)
 *     V = Constant               12
 *     Assignment,                 7
 *       with array element
 *     Assignment,                 6
 *       with record component
 *                                --
 *                                34       34
 *
 *     X = Y +|-|"&&"|"|" Z        5
 *     X = Y +|-|"==" Constant     6
 *     X = X +|- 1                 3
 *     X = Y *|/ Z                 2
 *     X = Expression,             1
 *           two operators
 *     X = Expression,             1
 *           three operators
 *                                --
 *                                18       18
 *
 *     if ....                    14
 *       with "else"      7
 *       without "else"   7
 *           executed        3
 *           not executed    4
 *     for ...                     7  |  counted every time
 *     while ...                   4  |  the loop condition
 *     do ... while                1  |  is evaluated
 *     switch ...                  1
 *     break                       1
 *     declaration with            1
 *       initialization
 *                                --
 *                                34       34
 *
 *     P (...)  procedure call    11
 *       user procedure      10
 *       library procedure    1
 *     X = F (...)
 *             function  call      6
 *       user function        5                                         
 *       library function     1                                               
 *                                --                                          
 *                                17       17
 *                                        ---
 *                                        103
 *
 *    The average number of parameters in procedure or function calls
 *    is 1.82 (not counting the function values aX *
 *
 *  2. Operators
 *  ------------
 *                          number    approximate
 *                                    percentage
 *
 *    Arithmetic             32          50.8                                 
 *
 *       +                     21          33.3                              
 *       -                      7          11.1                              
 *       *                      3           4.8
 *       / (int div)            1           1.6
 *
 *    Comparison             27           42.8
 *
 *       ==                     9           14.3
 *       /=                     4            6.3
 *       >                      1            1.6
 *       <                      3            4.8
 *       >=                     1            1.6
 *       <=                     9           14.3
 *
 *    Logic                   4            6.3
 *
 *       && (AND-THEN)          1            1.6
 *       |  (OR)                1            1.6
 *       !  (NOT)               2            3.2
 * 
 *                           --          -----
 *                           63          100.1
 *
 *
 *  3. Operand Type (counted once per operand reference):
 *  ---------------
 *                          number    approximate
 *                                    percentage
 *
 *     Integer               175        72.3 %
 *     Character              45        18.6 %
 *     Pointer                12         5.0 %
 *     String30                6         2.5 %
 *     Array                   2         0.8 %
 *     Record                  2         0.8 %
 *                           ---       -------
 *                           242       100.0 %
 *
 *  When there is an access path leading to the final operand (e.g. a record
 *  component), only the final data type on the access path is counted.       
 *
 *
 *  4. Operand Locality:                                                      
 *  -------------------
 *                                number    approximate
 *                                          percentage
 *
 *     local variable              114        47.1 %
 *     global variable              22         9.1 %
 *     parameter                    45        18.6 %
 *        value                        23         9.5 %
 *        reference                    22         9.1 %
 *     function result               6         2.5 %
 *     constant                     55        22.7 %
 *                                 ---       -------
 *                                 242       100.0 %
 *
 *
 *  The program does not compute anything meaningful, but it is syntactically
 *  and semantically correct. All variables have a value assigned to them
 *  before they are used as a source operand.
 *
 *  There has been no explicit effort to account for the effects of a
 *  cache, or to balance the use of long or short displacements for code or
 *  data.
 *
 ***************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Compiler and system dependent definitions: */
#define MSC_CLOCK

#ifndef TIME
#undef TIMES
#define TIMES
#endif
                /* Use times(2) time function unless    */
                /* explicitly defined otherwise         */

#ifdef MSC_CLOCK
#undef HZ
#undef TIMES
#include <time.h>
#ifdef KERNEL_RTTHREAD
#include <rtthread.h>
#define HZ        RT_TICK_PER_SECOND
#else
#define HZ        1000
#endif	// KERNEL_RTTHREAD
#endif
                /* Use Microsoft C hi-res clock */

#ifdef TIMES
#include <sys/types.h>
#include <sys/times.h>
                /* for "times" */
#endif

#define Mic_secs_Per_Second     1000000
                /* Berkeley UNIX C returns process times in seconds/HZ */

#ifdef  NOSTRUCTASSIGN
#define structassign(d, s)      memcpy(&(d), &(s), sizeof(d))
#else
#define structassign(d, s)      d = s
#endif

#ifdef  NOENUM
#define Ident_1 0
#define Ident_2 1
#define Ident_3 2
#define Ident_4 3
#define Ident_5 4
  typedef int   Enumeration;
#else
  typedef       enum    {Ident_1, Ident_2, Ident_3, Ident_4, Ident_5}
                Enumeration;
#endif
        /* for boolean and enumeration types in Ada, Pascal */

/* General definitions: */

#include <stdio.h>
                /* for strcpy, strcmp */

#define Null 0 
                /* Value of a Null pointer */
#define true  1
#define false 0

#define STATIC_DDR  0

#if !STATIC_DDR
#define INT_GLOB    Int_Glob
#define BOOL_GLOB   Bool_Glob
#define CH_1_GLOB   Ch_1_Glob
#define CH_2_GLOB   Ch_2_Glob

typedef int     Arr_2_Dim [50] [50];
#else
#define INT_GLOB    *Int_Glob
#define BOOL_GLOB   *Bool_Glob
#define CH_1_GLOB   *Ch_1_Glob
#define CH_2_GLOB   *Ch_2_Glob

typedef int     **Arr_2_Dim;
#endif

typedef int     One_Thirty;
typedef int     One_Fifty;
typedef char    Capital_Letter;
typedef int     Boolean;
typedef char    Str_30 [31];
typedef int     Arr_1_Dim [50];

typedef struct record 
    {
    struct record *Ptr_Comp;
    Enumeration    Discr;
    union {
          struct {
                  Enumeration Enum_Comp;
                  int         Int_Comp;
                  char        Str_Comp [31];
                  } var_1;
          struct {
                  Enumeration E_Comp_2;
                  char        Str_2_Comp [31];
                  } var_2;
          struct {
                  char        Ch_1_Comp;
                  char        Ch_2_Comp;
                  } var_3;
          } variant;
      } Rec_Type, *Rec_Pointer;

void Proc_1 (Rec_Pointer Ptr_Val_Par);
void Proc_2 (One_Fifty *Int_Par_Ref);
void Proc_3 (Rec_Pointer *Ptr_Ref_Par);
void Proc_4 (void);
void Proc_5 (void);
void Proc_6 (Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par);
void Proc_7 (One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty *Int_Par_Ref);
void Proc_8 (Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val);

Enumeration Func_1 (Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
Boolean Func_2 (Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);
Boolean Func_3 (Enumeration Enum_Par_Val);
