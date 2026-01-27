/*
* MessagesTask.cpp
*
*  Created on: 18.04.2020
*      Author: agorzawski
*/


#include "MessagesTask.h"
#include <DisplayTask.hpp>
#include <iterator>
#include "utils.h"
#include "tasks_utils.h"
#include "config.h"

#define DEFAULT_DISPLAY_TIME 0.05_s

MessagesTask::MessagesTask()
{
  addRegularMessage({this, [this](){return getMessages();}, DEFAULT_DISPLAY_TIME, 1, true});
}

void MessagesTask::run()
{
  updateFromConfig();
  sleep(60_s);
}

void MessagesTask::updateFromConfig()
{
  std::vector<String> new_message_keys;
  const auto& all_keys = DataStore::availableKeys();
  
  std::copy_if(all_keys.begin(), 
               all_keys.end(), 
               std::back_inserter(new_message_keys),
               [](const String& k) {return k.startsWith("messages.");});

  if (new_message_keys == messageKeys)
  {
    logPrintfX(F("MSG"), F("No new keys..."));
    return;
  }

  //new keys detected
  messageKeys = std::move(new_message_keys);
  messageCycleIndex = 0;
  
  logPrintfX(F("MSG"), F("Config for %zu message(s) found!"), messageKeys.size());
}

String MessagesTask::getMessages()
{
  String result;
  if (messageKeys.size() == 0)
    return result;

  if (DataStore::hasValue("messagesSplit"))
  {
    String messageSplit = DataStore::value("messagesSplit");
    std::vector<String> messageToReturn = {"All messages:", messageSplit};

    std::transform(messageKeys.begin(), messageKeys.end(),
                   std::back_inserter(messageToReturn),
                   [&messageToReturn, this, &messageSplit](const String& k) 
                   {return getMessage(k) + messageSplit;});

    result = getOneStringFrom(messageToReturn);
  }
  else
  {
    result = getMessage(messageKeys[messageCycleIndex]);

    messageCycleIndex++;
    if (messageCycleIndex >= messageKeys.size())
        messageCycleIndex = 0;
  }
  return result;
}

struct DeltaTimeReplacer
{
    DeltaTimeReplacer(time_t delta): delta(delta) {}

    String operator()(const char* t)
    {
      String tag = t;
      if (tag == "D") return formatDeltaTime(delta, DeltaTimePrecision::DAYS);
      if (tag == "H") return formatDeltaTime(delta, DeltaTimePrecision::HOURS);
      if (tag == "M") return formatDeltaTime(delta, DeltaTimePrecision::MINUTES);
      if (tag == "S") return formatDeltaTime(delta, DeltaTimePrecision::SECONDS);

      return dataSource(t);
    }

    time_t delta;
};


String MessagesTask::getMessage(String messageKey)
{
  String config = DataStore::valueOrDefault(messageKey, "...");

  std::vector<String> fields = tokenize(config,";");
  if (fields.size() != 3)
    return String();

  time_t when = fields[2].toInt();
  if (not when)
    return String();
  
  time_t delta = when - time(NULL);

  const String& before = fields[0];
  const String& after =  fields[1];

  String selected_string  = (delta > 0) ? before: after;

  StringStream ss;
  StringViewStream svs(selected_string);
  DeltaTimeReplacer dtr(delta);

  macroStringReplaceS(svs, dtr, ss);

  logPrintfX(F("MSG"), "%s", ss.buffer.c_str());

  return ss.buffer;
}

String MessagesTask::getOneStringFrom(std::vector<String> messages)
{
  int totalLen = 0;
  for (auto oneMsg : messages)
    totalLen += oneMsg.length();
  String toReturn;
  toReturn.reserve(totalLen+1);
  for (auto oneMsg : messages)
    toReturn.concat(oneMsg);
  return toReturn;
}
