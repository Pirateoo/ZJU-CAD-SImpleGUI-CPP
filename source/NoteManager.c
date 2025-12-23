#include "NoteManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 初始化笔记管理器
void initNoteManager(NoteManager* manager) {
    manager->head = NULL; // 初始化链表头指针为空
    manager->count = 0; // 初始化笔记数量为0
}

// 添加新笔记
void addNote(NoteManager* manager, const char* title, const char* content, const char* category) {
    // 为新笔记节点分配内存
    NoteNode* newNode = (NoteNode*)malloc(sizeof(NoteNode));
    if (newNode == NULL) {
        printf("Failed to allocate memory for new note.\n");
        exit(1);
    }
    // 复制笔记标题、内容和类别到新节点
    strncpy(newNode->note.title, title, MAX_TITLE_LENGTH);
    newNode->note.title[MAX_TITLE_LENGTH - 1] = '\0';
    strncpy(newNode->note.content, content, MAX_CONTENT_LENGTH);
    newNode->note.content[MAX_CONTENT_LENGTH - 1] = '\0';
    strncpy(newNode->note.category, category, MAX_CATEGORY_LENGTH);
    newNode->note.category[MAX_CATEGORY_LENGTH - 1] = '\0';
    time_t nowtime;
    time(&nowtime);
    newNode->note.createTime = nowtime; // 记录创建时间
    newNode->note.noteWordsCount = strlen(content); // 计算笔记字数
    // 将新节点插入链表头部
    newNode->next = manager->head;
    manager->head = newNode;
    manager->count++; // 增加笔记数量
}

// 根据标题删除笔记
void deleteNoteByTitle(NoteManager* manager, const char* title) {
    NoteNode* current = manager->head;
    NoteNode* previous = NULL;

    // 遍历链表查找匹配标题的笔记
    while (current != NULL && strcmp(current->note.title, title) != 0) {
        previous = current;
        current = current->next;
    }

    // 如果找到匹配的笔记节点
    if (current != NULL) {
        if (previous != NULL) {
            previous->next = current->next; // 从链表中移除该节点
        }
        else {
            manager->head = current->next; // 更新链表头指针
        }
        free(current); // 释放内存
        manager->count--; // 减少笔记数量
    }
    else {
        printf("Note with title '%s' not found.\n", title); // 未找到匹配的笔记
    }
}

// 根据标题查找笔记
Note* findNoteByTitle(NoteManager* manager, const char* title) {
    NoteNode* current = manager->head;
    // 遍历链表查找匹配标题的笔记
    while (current != NULL) {
        if (strcmp(current->note.title, title) == 0) {
            return &current->note; // 返回找到的笔记
        }
        current = current->next;
    }
    return NULL; // 未找到匹配的笔记
}

// 根据内容查找笔记
Note* findNoteByContent(NoteManager* manager, const char* content) {
	NoteNode* current = manager->head;
	// 遍历链表查找匹配内容的笔记
	while (current != NULL) {
		if (strcmp(current->note.content, content) == 0) {
			return &current->note; // 返回找到的笔记
		}
		current = current->next;
	}
	return NULL; // 未找到匹配的笔记
}

// 将所有笔记保存到文件
void saveNotesToFile(NoteManager* manager, const char* filename) {
    FILE* file = fopen(filename, "w"); // 以写模式打开文件
    if (file) {
        NoteNode* current = manager->head;
        // 遍历链表写入每个笔记到文件
        while (current != NULL) {
            fprintf(file, "title:%s\ncontent:%s\ncategory:%s\ncreateTime:%ld\n",
                current->note.title, current->note.content, current->note.category, (long)current->note.createTime);
            current = current->next;
        }
        fclose(file); // 关闭文件
    }
    else {
        printf("Failed to open file for writing.\n"); // 打开文件失败
    }
}

// 从文件加载笔记
void loadNotesFromFile(NoteManager* manager, const char* filename) {
    FILE* file = fopen(filename, "r"); // 以读模式打开文件
    initNoteManager(manager); // 初始化笔记管理器
    if (file) {
        char title[MAX_TITLE_LENGTH];
        char content[MAX_CONTENT_LENGTH];
        char category[MAX_CATEGORY_LENGTH];
        long createTime;
        // 逐个读取笔记并添加到管理器
        while (fscanf(file, "title:%[^\n]\ncontent:%[^\n]\ncategory:%[^\n]\ncreateTime:%ld\n",
            title, content, category, &createTime) == 4) {
            addNote(manager, title, content, category);
            NoteNode* newNode = manager->head;
            newNode->note.createTime = (time_t)createTime;
        }
        fclose(file); // 关闭文件
    }
    else {
        printf("Failed to open file for reading.\n"); // 打开文件失败
    }
}

// 修改笔记
void modifyNote(NoteManager* manager, Note* note, const char* newTitle, const char* newContent, const char* newCategory) {
    if (note) {
        // 更新笔记的标题、内容和类别
        strncpy(note->title, newTitle, MAX_TITLE_LENGTH);
        note->title[MAX_TITLE_LENGTH - 1] = '\0';
        strncpy(note->content, newContent, MAX_CONTENT_LENGTH);
        note->content[MAX_CONTENT_LENGTH - 1] = '\0';
        strncpy(note->category, newCategory, MAX_CATEGORY_LENGTH);
        note->category[MAX_CATEGORY_LENGTH - 1] = '\0';

        // Optionally update createTime if modification time is tracked
        time_t nowtime;
        time(&nowtime);
        note->createTime = nowtime;
        note->noteWordsCount = strlen(note->content); // 计算笔记字数
    }
}
