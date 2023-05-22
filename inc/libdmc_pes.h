#ifndef __libdmc_pes_h__
#define __libdmc_pes_h__

#ifdef __cplusplus
extern "C" {
#endif

    extern int dmc_pes_subscribe(int max_channel, char *ip, int port);
    extern int dmc_pes_unsubscribe(void);

#ifdef __cplusplus
}
#endif

#endif /*__libdmc_pes_h__*/
