#pragma once
class CTrain
{
public:
	CTrain(void);
	~CTrain(void);

	int excuteTrain();
	int excuteIndefine();

private:
	enum{
		INPUT_VECTOR_DIMS = 256,
		OUT_VECTOR_DIMS  =  36,
		CHARS_PER_COUNTS =  5,
		CHARS_CLASS_COUNTS = 36,
		CHARS_ALL_COUNTS  = (CHARS_PER_COUNTS * CHARS_CLASS_COUNTS),
	};

	void init();
	int preprocess(const char *path, int row, char c);
	void saveFeatures(void *pSrc, int row, char c);

	float   inputs[CHARS_ALL_COUNTS][INPUT_VECTOR_DIMS];
	float   outputs[CHARS_ALL_COUNTS][OUT_VECTOR_DIMS];
};

