
extern "C" __declspec(dllexport) bool translateJson(const char* json_file_path, const char* out_file_path);

// �˺�����Ҫ��translateJson֮��ʹ��
extern "C" __declspec(dllexport) int getCameraNumber();

// �˺�����Ҫ��translateJson֮��ʹ��
// outΪ�ⲿ�����ַ��������ڻ�ȡ��������֣����������ⲿ�����ڴ�
extern "C" __declspec(dllexport) int getCameraName(int num, char *out);
