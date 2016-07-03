/**
 * Based on alsactl/monitor.c (by Takashi Iwai <tiwai@suse.de>) from alsa-utils
 * http://git.alsa-project.org/?p=alsa-utils.git;a=blob_plain;f=alsactl/monitor.c;hb=HEAD
 *
 * Inspired by kbdd
 */

#include <stdio.h>
#include <alsa/asoundlib.h>
#include <dbus/dbus.h>

int notify(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0;

    msg = dbus_message_new_signal("/com/ch1p/Object", "com.ch1p.avm", "valueChanged");
    if (NULL == msg) {
        fprintf(stderr, "DBus: Message Null\n");
        return 1;
    }

    if (!dbus_connection_send(conn, msg, &serial)) {
        fprintf(stderr, "DBus send: Out Of Memory!\n");
        return 1;
    }
    dbus_connection_flush(conn);

    dbus_message_unref(msg);
    return 0;
}

int open_ctl(const char *name, snd_ctl_t **ctlp)
{
	snd_ctl_t *ctl;
	int err;

	err = snd_ctl_open(&ctl, name, SND_CTL_READONLY);
	if (err < 0) {
		fprintf(stderr, "Cannot open ctl %s\n", name);
		return err;
	}
	err = snd_ctl_subscribe_events(ctl, 1);
	if (err < 0) {
		fprintf(stderr, "Cannot open subscribe events to ctl %s\n", name);
		snd_ctl_close(ctl);
		return err;
	}
	*ctlp = ctl;
	return 0;
}

int check_event(snd_ctl_t *ctl, DBusConnection *conn)
{
	snd_ctl_event_t *event;
	unsigned int mask;
	int err;

	snd_ctl_event_alloca(&event);
	err = snd_ctl_read(ctl, event);
	if (err < 0)
		return err;

	if (snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM)
		return 0;
	
    mask = snd_ctl_event_elem_get_mask(event);
    if (!(mask & SND_CTL_EVENT_MASK_VALUE))
        return 0;

    notify(conn);

	return 0;
}

#define MAX_CARDS	256

int monitor(const char *name, DBusConnection *conn) {
    snd_ctl_t *ctl;
	int err = 0;

    err = open_ctl(name, &ctl);
    if (err < 0)
        goto error;

    while (1) {
		struct pollfd fd;

		snd_ctl_poll_descriptors(ctl, &fd, 1);

		err = poll(&fd, 1, -1);
		if (err <= 0) {
			err = 0;
			break;
		}

        unsigned short revents;
        snd_ctl_poll_descriptors_revents(ctl, &fd, 1,
                         &revents);
        if (revents & POLLIN)
            check_event(ctl, conn);
	}

error:
	snd_ctl_close(ctl);
	return err;
}

void usage()
{
    fprintf(stderr,
        "Usage:\n"
        "   alsa-volume-monitor <card name>\n\n"
        "Example:\n"
        "   alsa-volume-monitor hw:0\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        usage();

    DBusConnection *connection;
    DBusError err;
    int ret;

    dbus_error_init(&err);
    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (!connection) {
        return 1;
    }

    ret = dbus_bus_request_name(connection, "com.ch1p.avm",
        DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    monitor(argv[1], connection);

    return 0;
}
