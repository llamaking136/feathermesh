/* Automatically generated nanopb header */
/* Generated by nanopb-1.0.0-dev */

#ifndef PB_MESHTASTIC_MESHTASTIC_RTTTL_PB_H_INCLUDED
#define PB_MESHTASTIC_MESHTASTIC_RTTTL_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* Canned message module configuration. */
typedef struct _meshtastic_RTTTLConfig {
    /* Ringtone for PWM Buzzer in RTTTL Format. */
    char ringtone[231];
} meshtastic_RTTTLConfig;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define meshtastic_RTTTLConfig_init_default      {""}
#define meshtastic_RTTTLConfig_init_zero         {""}

/* Field tags (for use in manual encoding/decoding) */
#define meshtastic_RTTTLConfig_ringtone_tag      1

/* Struct field encoding specification for nanopb */
#define meshtastic_RTTTLConfig_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   ringtone,          1)
#define meshtastic_RTTTLConfig_CALLBACK NULL
#define meshtastic_RTTTLConfig_DEFAULT NULL

extern const pb_msgdesc_t meshtastic_RTTTLConfig_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define meshtastic_RTTTLConfig_fields &meshtastic_RTTTLConfig_msg

/* Maximum encoded size of messages (where known) */
#define MESHTASTIC_MESHTASTIC_RTTTL_PB_H_MAX_SIZE meshtastic_RTTTLConfig_size
#define meshtastic_RTTTLConfig_size              233

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
