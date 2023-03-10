/*
 * TimeResolver.cpp
 *****************************************************************************
 * Copyright (C) 2013, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "TimeResolver.h"

using namespace libdash::framework::mpd;

bool                        TimeResolver::CheckTimeInterval   (std::string availabilityStartTime, std::string availabilityEndTime)
{
    struct tm startTime, *brokenTime;//  =
    struct tm currentTime;//  = TimeResolver::GetCurrentUTCTime();
    struct tm  endTime;//      = TimeResolver::ResolveUTCDateTime(availabilityEndTime);
    bool haveStart = false, haveEnd = false;

    brokenTime = TimeResolver::ResolveUTCDateTime(availabilityStartTime);
    if (brokenTime)
    {
        haveStart = true;
        memcpy(&startTime, brokenTime, sizeof(startTime));
      //  dash_log("availabilityStartTime:%s->[%d-%02d-%02d %d:%d:%d] ->%u\n", availabilityStartTime.c_str(),
      //      startTime.tm_year+1900, startTime.tm_mon+1, startTime.tm_mday, startTime.tm_hour,
       //     startTime.tm_min, startTime.tm_sec, mktime(&startTime));
    }

    brokenTime = TimeResolver::GetCurrentUTCTime();
    if (brokenTime)
    {
        memcpy(&currentTime, brokenTime, sizeof(currentTime));
     //   dash_log("currentTime:->[%d-%02d-%02d %d:%d:%d] ->%u\n",
     //       currentTime.tm_year+1900, currentTime.tm_mon+1, currentTime.tm_mday, currentTime.tm_hour,
      //      currentTime.tm_min, currentTime.tm_sec, mktime(&currentTime));
    }
    else
    {
        dash_log(DASH_LOG_ERROR, "[%s,%d] fail to GetCurrentUTCTime\n", __FUNCTION__, __LINE__);
        return true;
    }

    brokenTime = TimeResolver::ResolveUTCDateTime(availabilityEndTime);
    if (brokenTime)
    {
        haveEnd = true;
        memcpy(&endTime, brokenTime, sizeof(endTime));
    }

#if 0
    if (haveEnd)
    {
        dash_log(DASH_LOG_ERROR, "availabilityEndTime:%s->[%d-%02d-%02d %d:%d:%d] ->%u\n", availabilityEndTime.c_str(),
            endTime.tm_year+1900, endTime.tm_mon+1, endTime.tm_mday, endTime.tm_hour,
            endTime.tm_min, endTime.tm_sec, mktime(&endTime));
    }

   dash_log(DASH_LOG_ERROR, "currentTime [%d-%02d-%02d %d:%d:%d] ->%u\n",
    currentTime.tm_year+1900, currentTime.tm_mon+1, currentTime.tm_mday, currentTime.tm_hour,
    currentTime.tm_min, currentTime.tm_sec, mktime(&currentTime));
#endif

    if (!haveEnd)
        return true;

    if (difftime(mktime(&endTime),mktime(&currentTime)) > 0)
        return true;

    return false;
}
uint32_t                    TimeResolver::GetCurrentTimeInSec ()
{
    return mktime(TimeResolver::GetCurrentUTCTime());
}
uint32_t                    TimeResolver::GetUTCDateTimeInSec (const std::string& datetime)
{
    return mktime(TimeResolver::ResolveUTCDateTime(datetime));
}
double                      TimeResolver::GetDurationInSec    (const std::string& duration)
{
    /* no check for duration with yyyy,dd,mm */
    if (duration == "" || duration.substr(0, 2) != "PT")
        return 0;

    size_t      startPos    = 2;
    size_t      endPos      = std::string::npos;
    uint32_t    hours       = 0;
    uint32_t    mins        = 0;
    double      secs        = 0;

    char designators[] = { 'H', 'M', 'S' };

    endPos = duration.find(designators[0], startPos);
    if (endPos != std::string::npos)
    {
        hours = strtol(duration.substr(startPos, endPos - startPos).c_str(), NULL, 10);
        startPos = endPos + 1;
    }

    endPos = duration.find(designators[1], startPos);
    if (endPos != std::string::npos)
    {
        mins = strtol(duration.substr(startPos, endPos - startPos).c_str(), NULL, 10);
        startPos = endPos + 1;
    }

    endPos = duration.find(designators[2], startPos);
    if (endPos != std::string::npos)
        secs = strtod(duration.substr(startPos, endPos - startPos).c_str(), NULL);

    return hours*3600 + mins*60 + secs;
}

struct tm*                  TimeResolver::ResolveUTCDateTime  (const std::string& dateTimeString)
{
    if (dateTimeString == "")
        return NULL;

    time_t rawtime;
    struct tm* timeInfo;
    time ( &rawtime );
    timeInfo = gmtime ( &rawtime );

    std::string timeString = dateTimeString.substr();

    timeString = timeString.substr(0, timeString.size()-1);

    std::vector<std::string> dateTime   = SplitToStr(timeString, 'T');
    std::vector<int>         dateChunks = SplitToI(dateTime.at(0), '-');
    std::vector<int>         timeChunks = SplitToI(dateTime.at(1), ':');

    timeInfo->tm_year = dateChunks.at(0) - 1900;
    timeInfo->tm_mon  = dateChunks.at(1) - 1;
    timeInfo->tm_mday = dateChunks.at(2);

    timeInfo->tm_hour = timeChunks.at(0);
    timeInfo->tm_min  = timeChunks.at(1);
    timeInfo->tm_sec  = timeChunks.at(2);

    return timeInfo;
}
struct tm*                  TimeResolver::GetCurrentUTCTime   ()
{
    time_t      rawTime;

    time(&rawTime);
    return gmtime(&rawTime);
}
std::vector<int>            TimeResolver::SplitToI            (const std::string &s, char delim)
{
    std::stringstream   ss(s);
    std::string         item;
    std::vector<int>    integers;

    while(std::getline(ss, item, delim))
        integers.push_back((int)strtol(item.c_str(), NULL, 10));

    return integers;
}
std::vector<std::string>    TimeResolver::SplitToStr          (const std::string &s, char delim)
{
    std::stringstream           ss(s);
    std::string                 item;
    std::vector<std::string>    strings;

    while(std::getline(ss, item, delim))
        strings.push_back(item);

    return strings;
}
