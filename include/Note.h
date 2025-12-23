#ifndef NOTE_H
#define NOTE_H
#include<time.h>
// 定义最大标题长度
#define MAX_TITLE_LENGTH 100
// 定义最大内容长度
#define MAX_CONTENT_LENGTH 1000
// 定义最大类别长度
#define MAX_CATEGORY_LENGTH 50

// 定义笔记结构体
typedef struct Note {
    char title[MAX_TITLE_LENGTH];      // 笔记标题
    char content[MAX_CONTENT_LENGTH];  // 笔记内容
    char category[MAX_CATEGORY_LENGTH];// 笔记类别（新增字段）
    int noteWordsCount;			   // 笔记字数（新增字段）
    time_t createTime;           // 创建时间
} Note;

#endif // NOTE_H
