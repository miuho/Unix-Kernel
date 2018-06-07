/**
 * @file kern/inc/reporter.h
 * @brief header file for reporter to print debugging messages to
 *        simics4
 *
 * @author HingOn Miu (hmiu@andrew.cmu.edu)
 * @author An Wu (anwu@andrew.cmu.edu)
 *
 */


#ifndef _KERN_INC_REPORTER_H_
#define _KERN_INC_REPORTER_H_

#define ARG_ERR (-1)
#define MEM_ERR (-2)
#define STATE_ERR (-3)

/* verbose level:
 * 4: report everything
 * 3: report errors and warnings and progresses
 * 2: report errors and warnings
 * 1: report errors
 * 0: report nothing
 */
#define VERBOSE_LEVEL 0

/**
 * @brief print the misc type messages to simics4
 *
 * @param tag the location of function
 * @param fmt the string to print on simics4 
 * @return Void
 *
 */
void report_misc(const char *tag, const char *fmt, ...);

/**
 * @brief print the progress type messages to simics4
 *
 * @param tag the location of function
 * @param fmt the string to print on simics4 
 * @return Void
 *
 */
void report_progress(const char *tag, const char *fmt, ...);

/**
 * @brief print the report type messages to simics4
 *
 * @param tag the location of function
 * @param fmt the string to print on simics4 
 * @return Void
 *
 */
void report_warning(const char *tag, const char *fmt, ...);

/**
 * @brief print the error type messages to simics4
 *
 * @param tag the location of function
 * @param fmt the string to print on simics4 
 * @return Void
 *
 */
void report_error(const char *tag, const char *fmt, ...);


#endif
