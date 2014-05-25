#ifndef _Inarray
#define _Inarray
#include"BaseClass.h"
#include"savekit.h"
#include"loadkit.h"
//�����ʾһ����len��������ɵ����飬ÿ������Ԫ�صĴ�СΪsizeλ�������������е�ֵ�ķ�ΧΪ0~len-1����
//size �Ĵ�СΪlog(len)�������ݷ�ΧΪ1~len����size�Ĵ�СΪlog(len)+1������Ĭ�ϵ������±��0��ʼ��
class InArray
{
public:
	InArray();
	InArray(i32 len,i32 size);
	~InArray(void);
	i32 GetValue(i32 index);
	void SetValue(i32 index,i32 value);
	i32 GetNum();
	i32 GetDataWidth();
	i32 GetMemorySize();
    i32 GetValue2(i32 index);
	i32 write(savekit & s);
	i32 load(loadkit & s);
private:
	u32 * data;
    i32 datanum;
	i64 datawidth;
	u32 mask;
};
#endif
