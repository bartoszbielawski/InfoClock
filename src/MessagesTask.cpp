/*
* MessagesTask.cpp
*
*  Created on: 18.04.2020
*      Author: agorzawski
*/


#include "MessagesTask.h"
#include <DisplayTask.hpp>
#include "utils.h"
#include "tasks_utils.h"
#include "config.h"

#define DEFAULT_DISPLAY_TIME 0.05_s

MessagesTask::MessagesTask(){
  if (DataStore::value("messagesEnabled").toInt())
  {
    updateFromConfig(true);
    addRegularMessage({this, [this](){return getMessages();}, DEFAULT_DISPLAY_TIME, 1, true});
  }
}

void MessagesTask::run()
{
  updateFromConfig(false);
  sleep(5_s);
}

void MessagesTask::updateFromConfig(bool verbose)
{
  std::vector<String> keys = DataStore::availableKeys();
  messageKeys.clear();

  for (String &key : keys)
  {
    if (key.startsWith("messages")) {
      std::vector<String> subKeys = tokenize(key, ".");
      if (subKeys.size() > 1)
      {
        if (std::find(messageKeys.begin(), messageKeys.end(), subKeys[1]) != messageKeys.end())
          continue;
        messageKeys.push_back(subKeys[1]);
      }
    }
  }
  if (verbose){logPrintfX(F("MSG"), F("Config for %d message(s) found!"), messageKeys.size());}
}

String MessagesTask::getMessages()
{
  if (DataStore::hasValue("messagesSplit"))
  {
    String messageSplit = DataStore::value("messagesSplit");
    std::vector<String> messageToReturn;
    messageToReturn.push_back("All messages:" + messageSplit);
    for(auto messageKey : messageKeys)
      messageToReturn.push_back(getMessage(messageKey) + messageSplit);
    return getOneStringFrom(messageToReturn);
  }
  else
  {
    messageCycleIndex++;
    if (messageCycleIndex < 0 || messageCycleIndex >= messageKeys.size())
      messageCycleIndex = 0;
    return getMessage(messageKeys[messageCycleIndex]);
  }
}

String MessagesTask::getMessage(String messageKey)
{
  bool msgVerbose = (bool) DataStore::valueOrDefault("messagesVerbose", "0").toInt();
  String messageFullKey = "messages." + messageKey;
  String messageText = "[$KEY]" + DataStore::valueOrDefault(messageFullKey, "...$1...");

  time_t when = DataStore::valueOrDefault(messageFullKey + ".time", "0").toInt(); // TODO fix time_now to str_date
  bool countdown = (bool)  DataStore::valueOrDefault(messageFullKey + ".countdown", "1").toInt();

  DeltaTimePrecision precision = allowedPrecisions[0];
  int precisionId = DataStore::valueOrDefault(messageFullKey + ".precision", "0").toInt();
  if (precisionId > 0 &&  precisionId <= 4)
    precision = allowedPrecisions[precisionId];

  time_t delta = time(NULL) - when;
  if ((delta < 0 && !countdown) || (delta > 0 && countdown) )
      if (msgVerbose)
        return "[" + messageKey + "]" + defaultMessage;
      else
        return defaultMessage;

  if (msgVerbose)
    messageText.replace("$KEY", messageKey);
  else
    messageText.replace("[$KEY]", "");
  messageText.replace(defaultReplaceString, formatDeltaTime(delta, precision));
  return messageText;
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
