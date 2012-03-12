//! Time and date manipulation functions.
//! Most of these are copied from miotime.c in the original standard library

#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "openTuringLibDefs.h"
#include "openTuringRuntimeError.h"

/*************/
/* Constants */
/*************/
#define DATE_STR_LEN		9
#define TIME_STR_LEN		8
#define DATETIME_STR_LEN	DATE_STR_LEN + TIME_STR_LEN + 1
#define SPACE			' '

/********************/
/* Static constants */
/********************/
static const char	*stMonths[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	  			  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/*********************/
/* Static procedures */
/*********************/
static int	MyConvertDateStrToSeconds (char *pmDateString)
{
    struct tm	myTimeRecord;
    char 	myDateString [DATE_STR_LEN + 1];
    int		cnt;

    if ((strlen (pmDateString) >= DATE_STR_LEN) && 
    	(pmDateString [2] == SPACE) && (pmDateString [6] == SPACE)) 
    {
	strncpy (myDateString, pmDateString, DATE_STR_LEN);
	myDateString [DATE_STR_LEN] = 0;
    }
    else
    {
	return -1;
    }
    
    if (!((isdigit (myDateString [0]) || (myDateString [0] == SPACE)) && 
    	  isdigit (myDateString [1]) && isdigit (myDateString [7]) && 
    	  isdigit (myDateString [8])))
    {
	return -1;
    }

    myTimeRecord.tm_hour = 0;
    myTimeRecord.tm_min  = 0;
    myTimeRecord.tm_sec  = 0;
    myTimeRecord.tm_isdst = -1;

    // Convert the month
    for (cnt = 0; cnt < 12; cnt++) 
    {
	char myString [5];

	myTimeRecord.tm_mon = cnt;

	/* Test against english */
	if (strncmp (myDateString + 3, stMonths [cnt], 3) == 0)
	{
	    break;
	}
	strftime (myString, sizeof (myString), "%h", &myTimeRecord);

	/* Test against locale's month abbreviation */
	if (strncmp (myDateString + 3, myString, 3) == 0)
	{
	    break;
	}
    }

    if (cnt == 12) 
    {
	return -1;
    }

    myTimeRecord.tm_mday = 
    	atoi ((myDateString[0] == SPACE) ? myDateString + 1 : myDateString);
    myTimeRecord.tm_year = atoi (myDateString + 7);

    if (myTimeRecord.tm_year < 70)
    {
	myTimeRecord.tm_year += 100;
    }

    return (int) mktime (&myTimeRecord);
} // MyConvertDateStrToSeconds

static int	MyConvertTimeStrToSeconds (char *pmTimeString)
{
    if ((pmTimeString [2] != ':') || (pmTimeString[5] != ':') || 
        (strlen (pmTimeString) != TIME_STR_LEN))
    {
	return -1;
    }

    if (!(isdigit (pmTimeString [0]) && isdigit (pmTimeString [1]) && 
    	  isdigit (pmTimeString [3]) && isdigit (pmTimeString [4]) && 
    	  isdigit (pmTimeString [6]) && isdigit (pmTimeString [7])))
    {
	return -1;
    }

    return atoi (pmTimeString) * 3600 + atoi (pmTimeString + 3) * 60 + 
    	   atoi (pmTimeString + 6);
} // MyConvertTimeStrToSeconds

extern "C" {

	/************************************************************************/
	/* Turing_StdlibSFML_Time_DateSec							*/
	/*									*/
	/* Parse a string, converting it into OOT time (perhaps relative)	*/
	/* String can be in one of three forms:					*/
	/*									*/
	/* "dd mmm yy"		  - returns ootTime based on midnight		*/
	/* "hh:mm:ss"		  - returns seconds since midnight		*/
	/* "dd mmm yy hh:mm:ss"   - returns ootTime				*/
	/************************************************************************/
	TInt	Turing_StdlibSFML_Time_DateSec (TString *pmTuringDateTimeStr)
	{
	    int myOOTTime;
	    char *pmDateTimeStr = pmTuringDateTimeStr->strdata;

	    /* Ignore white space at beginning */

	    if (*pmDateTimeStr == 0) 
	    {
			turingRuntimeError("Invalid time string.");
	    }

	    if (pmDateTimeStr [2] == ':') 
	    {
		/* Only time (possibly) */
		myOOTTime = MyConvertTimeStrToSeconds (pmDateTimeStr);

		if (myOOTTime == -1) 
		{
		    turingRuntimeError("Invalid time string.");
		}
	    }
	    else 
	    {
	        char *myTimeStr = pmDateTimeStr;

		myOOTTime = MyConvertDateStrToSeconds (myTimeStr);

		if (myOOTTime == -1) 
		{
		    turingRuntimeError("Invalid time string.");
		}

		myTimeStr += DATE_STR_LEN;

		if (*myTimeStr) 
		{
		    int myTimeSecs = MyConvertTimeStrToSeconds (myTimeStr + 1);

		    if (myTimeSecs == -1) 
		    {
		        turingRuntimeError("Invalid time string.");
		    }
		    myOOTTime += myTimeSecs;
		}
	    }
	    return myOOTTime;
	} // Turing_StdlibSFML_Time_DateSec

	TInt	Turing_StdlibSFML_Time_PartsSec (TInt pmYear, TInt pmMon, TInt pmDay, 
			          TInt pmHour, TInt pmMin, TInt pmSec)
	{
	    struct tm	myTimeRecord;

	    myTimeRecord.tm_mday = pmDay;
	    myTimeRecord.tm_mon = pmMon - 1;
	    myTimeRecord.tm_year = pmYear - 1900;

	    myTimeRecord.tm_hour = pmHour;
	    myTimeRecord.tm_min = pmMin;
	    myTimeRecord.tm_sec = pmSec;

	    myTimeRecord.tm_isdst = -1;

	    return (TInt) mktime (&myTimeRecord);
	} // Turing_StdlibSFML_Time_PartsSec

	TInt	Turing_StdlibSFML_Time_Sec (void)
	{
	    return (TInt) time ((time_t *)NULL);
	} // Turing_StdlibSFML_Time_Sec

	void	Turing_StdlibSFML_Time_SecDate (TString *pmTuringTimeStr, TInt pmOOTTime)
	{
		char *pmOOTimeStr = pmTuringTimeStr->strdata;
	    if (pmOOTTime == -1)
	    {
		pmOOTimeStr [0] = 0;
		turingRuntimeError("Seconds time out of range. Maybe you used a date too far in the past or future.");
	    }
	    else
	    {
		time_t epochTime = pmOOTTime;
		struct tm *myTimeRec = localtime ((time_t *) &epochTime);
		
		strftime (pmOOTimeStr, DATETIME_STR_LEN + 1, "%d %b %y %H:%M:%S", myTimeRec);
		
		if (pmOOTimeStr[0] == '0')
		{
		    pmOOTimeStr[0] = SPACE;
		}
	    }
	} // Turing_StdlibSFML_Time_SecDate


	void	Turing_StdlibSFML_Time_SecParts (TInt pmOOTTime, TInt *pmYear, TInt *pmMon, 
				  TInt *pmDay, TInt *pmDOW, TInt *pmHour, 
			          TInt *pmMin, TInt *pmSec)
	{
		time_t epochTime = pmOOTTime;
	    struct tm	*myTimeRecord = localtime ((time_t *) &epochTime);

	    *pmYear = myTimeRecord -> tm_year + 1900;
	    *pmMon = myTimeRecord -> tm_mon + 1;
	    *pmDay = myTimeRecord -> tm_mday;

	    *pmDOW = myTimeRecord -> tm_wday + 1;

	    *pmHour = myTimeRecord -> tm_hour;
	    *pmMin = myTimeRecord -> tm_min;
	    *pmSec = myTimeRecord -> tm_sec;
	} // Turing_StdlibSFML_Time_SecParts


	void	Turing_StdlibSFML_Time_SecStr (TString *pmString, TInt pmOOTTime, 
			        TString *pmFormatStr)
	{
		time_t epochTime = pmOOTTime;
	    struct tm	*myTimeRecord = localtime ((time_t *) &epochTime);

	    strftime (pmString->strdata, TURING_STRING_LENGTH, pmFormatStr->strdata, myTimeRecord);
	} // Turing_StdlibSFML_Time_SecStr
    
    void Turing_StdlibSFML_Time_Date (TString *pmOOTimeStr)
	{
		TInt epochTime;
		epochTime = Turing_StdlibSFML_Time_Sec ();
	    Turing_StdlibSFML_Time_SecDate (pmOOTimeStr, epochTime);
	} // Turing_StdlibSFML_Time_Date
}