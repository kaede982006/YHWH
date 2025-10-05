#ifndef YHWH_TYPE
#define YHWH_TYPE

typedef unsigned char Y_BYTE;


#pragma pack(push,1)
typedef struct { unsigned char r, g, b; } S_YHWH_RGB;
#pragma pack(pop)

typedef struct {
	int duration;
	S_YHWH_RGB* buffer;
} S_YHWH_GRAPHIC;

typedef struct {
	BITMAPINFOHEADER bi;
	DWORD masks[3]; // R,G,B
} BMI32;

typedef enum _YHWH_ACTION
{
	YHWHNoReboot,
	YHWHReboot,
	YHWHPowerOff
} YHWH_ACTION, * PYHWH_ACTION;

#endif
