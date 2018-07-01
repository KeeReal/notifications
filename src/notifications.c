#include "main.h"

void notification_close(void) {
    if (root.notification) {
        notify_notification_close(root.notification, NULL);
    }
}

void notification_show(config_t *config) {
    if (root.notification == NULL) {
        root.notification = notify_notification_new(NULL, NULL, NULL);
    }
    notify_notification_update(root.notification,
                               NOTIFICATIONS_SUMMARY,
                               config->message,
                               NOTIFICATIONS_ICON);
    notify_notification_show(root.notification, NULL);
}

