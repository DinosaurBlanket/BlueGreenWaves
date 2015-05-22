
__kernel void helloPixel(
	float curFrame,
	__write_only image2d_t out
) {
	const int2 pos = {get_global_id(0), get_global_id(1)};
	const int2 dim = {get_global_size(0), get_global_size(1)};
	float4 color;
	// xyzw : abgr
	color.x = 1.0f;
	color.y = sinpi(
		( (pos.x + fmod(curFrame*8, dim.x)) / dim.x ) * 2.0f
	) / 2.0f + 0.5f;
	color.z = sinpi(
		( (pos.y + fmod(curFrame, dim.y)) / dim.y ) * 2.0f
	) / 2.0f + 0.5f;
	color.w = 0;
	write_imagef(out, pos, color);
}
