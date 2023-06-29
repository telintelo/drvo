#ifndef __DRVO_FINAL_H__
#define __DRVO_FINAL_H__

#define DRVO_LOGGING_PREFIX "DRVO-FINAL: "
#define DRVO_emerg(fmt,arg...) \
    pr_emerg(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_alert(fmt,arg...) \
    pr_alert(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_crit(fmt,arg...) \
    pr_crit(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_err(fmt,arg...) \
    pr_err(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_warn(fmt,arg...) \
    pr_warn(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_notice(fmt,arg...) \
    pr_notice(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_info(fmt,arg...) \
    pr_info(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_devel(fmt,arg...) \
    pr_devel(DRVO_LOGGING_PREFIX fmt,##arg)
#define DRVO_debug(fmt,arg...) \
    pr_debug(DRVO_LOGGING_PREFIX fmt,##arg)

#endif /* __DRVO_FINAL_H__ */
