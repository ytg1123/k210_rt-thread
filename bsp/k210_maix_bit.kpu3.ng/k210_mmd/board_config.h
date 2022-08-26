#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#if (OV5640 && OV2640) || (!OV5640 && !OV2640)
#error ov sensor only choose one
#endif

#if (BOARD_LICHEEDAN && BOARD_KD233) || (BOARD_LICHEEDAN && BOARD_K61) || (BOARD_K61 && BOARD_KD233) || (!BOARD_LICHEEDAN && !BOARD_KD233 && !BOARD_K61)
#error board only choose one
#endif

#endif
