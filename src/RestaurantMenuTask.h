/*
 * RestaurantMenuTask.h
 *
 *  Created on: 27.07.2025
 *      Author: Bauke Spoelstra
 */

#ifndef RESTAURANTMENUTASK_H_
#define RESTAURANTMENUTASK_H_

#include "tasks_utils.h"
#include "DisplayTask.hpp"
#include <vector>
#include <set>
#include <ESP8266WebServer.h>

namespace Tasks {

class RestaurantMenuTask : public Task {
public:
    RestaurantMenuTask();

    virtual void run() override;
    String getMenuString() const;

    void handleStatusPage(ESP8266WebServer& webServer);

private:
    void fetchMenu(const String& dateStr);
    String makeMenuDateString(time_t base) const;
    void updateMenuHoursFromConfig();
    bool isWithinDisplayHour() const;

    int restaurantCode = 1;
    String restaurantId = "33-restaurant-r3";
    String novaeKey = "CER103";

    int menuStartHour = 9;
    int menuEndHour = 14;
    bool menuShowTomorrow = false;

    String cachedMenuDate;
    String cachedMenuLine;
    String lastFetchedMenuDate;
    int lastFetchHour = -1;

    std::vector<String> dishes;

    // Status page fields
    String lastStatusTimestamp;
};

} // namespace Tasks

#endif // RESTAURANTMENUTASK_H_
