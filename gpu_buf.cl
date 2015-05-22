
__kernel void helloPixel(
	float videoWidth,
	float videoHeight,
	float curFrame,
	__global uchar4 *out
) {
	uint gid = get_global_id(0);
	float row = gid/videoWidth;
	float col = fmod(gid, videoWidth);
	// xyzw : abgr
	out[gid].x = 255;
	out[gid].y = 255 * (
		sinpi(
			( (col + fmod(curFrame*8, videoWidth)) / videoWidth ) * 2.0
		) / 2.0 + 0.5
	);
	out[gid].z = 255 * (
		sinpi(
			( (row + fmod(curFrame, videoHeight)) / videoHeight ) * 2.0
		) / 2.0 + 0.5
	);
	out[gid].w = 0;
}
