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

  for (String &key : keys)
  {
    if (key.startsWith("messages")) {
      std::vector<String> subKeys = tokenize(key, ".");
      // TODO make it safer (checks) + Bartosz idea
      if (subKeys.size() > 1)
      {
        nbOfMessages = subKeys[1].toInt();
      }
    }
  }
  if (verbose){logPrintfX(F("MSG"), F("Config for %d message(s) found!"), nbOfMessages);}

}

String MessagesTask::getMessages()
{
  if (DataStore::hasValue("messagesSplit"))
  {
    String messageSplit = DataStore::value("messagesSplit");
    String toReturn = messageSplit;
    for (int i=1; i<=nbOfMessages; i++)
    {
      toReturn = toReturn + getMessage(i) + messageSplit;
    }
    return toReturn;
  }
  else
  {
    messageCycleIndex++;
    if (messageCycleIndex < 1 || messageCycleIndex > nbOfMessages) messageCycleIndex=1;
      return getMessage(messageCycleIndex);
  }
}

String MessagesTask::getMessage(int messageIndex)
{
  bool countdown = defaultCounting;
  DeltaTimePrecision precision = defaultPrecision;
  char msgkey[128];
  snprintf(msgkey, sizeof(msgkey), "messages.%d", messageIndex);
  String messageText = DataStore::value(msgkey);
  snprintf(msgkey, sizeof(msgkey), "messages.%d.time", messageIndex);
  time_t when = DataStore::value(msgkey).toInt();
  snprintf(msgkey, sizeof(msgkey), "messages.%d.countdown", messageIndex);
  if (DataStore::hasValue(msgkey)){ countdown = (bool) DataStore::value(msgkey).toInt();}
  snprintf(msgkey, sizeof(msgkey), "messages.%d.precision", messageIndex);
  if (DataStore::hasValue(msgkey))
  {
    switch (DataStore::value(msgkey).toInt())
    {
      case 0: precision = DeltaTimePrecision::DAYS; break;
      case 1: precision = DeltaTimePrecision::HOURS; break;
      case 2: precision = DeltaTimePrecision::MINUTES; break;
      case 3: precision = DeltaTimePrecision::SECONDS; break;
    }
  }


  time_t delta = time(NULL) - when;
  if ((delta < 0 && !countdown) || (delta > 0 && countdown) )
      return defaultMessage;

  char msgtoReturn[128];
  snprintf(msgtoReturn, sizeof(msgtoReturn), messageText.c_str(), formatDeltaTime(delta, precision).c_str());
  return msgtoReturn;
}
