
extern "C" __declspec(dllexport) bool translateJson(const char* json_file_path, const char* out_file_path);

// 此函数需要在translateJson之后使用
extern "C" __declspec(dllexport) int getCameraNumber();

// 此函数需要在translateJson之后使用
// out为外部传入字符串，用于获取摄像机名字，并且需在外部分配内存
extern "C" __declspec(dllexport) int getCameraName(int num, char *out);
