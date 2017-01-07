//  clockslave.c - clock synchronization test/demo
////
//  see clockmaster.c for details


#include "o2.h"
#include "stdio.h"
#include "string.h"

#ifdef WIN32
#include <windows.h> 
#else
#include <unistd.h>
#endif

o2_time cs_time = 1000000.0;

void clockslave(o2_msg_data_ptr msg, const char *types,
                o2_arg ** argv, int argc, void *user_data)
{
    int ss = o2_status("server");
    int cs = o2_status("client");
    double mean_rtt, min_rtt;
    o2_roundtrip(&mean_rtt, &min_rtt);
    printf("clockslave: local time %g global time %g "
           "ss %d cs %d mean %g min %g\n",
           o2_local_time(), o2_time_get(), ss, cs, mean_rtt, min_rtt);
    if (ss == O2_REMOTE) {
        if (o2_time_get() < cs_time) cs_time = o2_time_get();
    }
    // stop 10s later
    if (o2_time_get() + 10 > cs_time) o2_stop_flag = TRUE;
    // Since the clock slave cannot immediately send scheduled messages
    // due to there being no global time reference, we will schedule
    // messages directly on the local scheduler
    o2_send_start();
    o2_message_ptr m = o2_message_finish(o2_local_time() + 1,
                                         "!client/clockslave", TRUE);
    o2_schedule(&o2_ltsched, m);
}


int main(int argc, const char * argv[])
{
    o2_initialize("test");
    o2_service_new("client");
    o2_method_new("/client/clockslave", "", &clockslave, NULL, FALSE, FALSE);
    // this particular handler ignores all parameters, so this is OK:
    // start polling and reporting status
    clockslave(NULL, NULL, NULL, 0, NULL);
    o2_run(100);
    o2_finish();
    sleep(1);
    printf("CLOCKSLAVE DONE\n");
    return 0;
}
