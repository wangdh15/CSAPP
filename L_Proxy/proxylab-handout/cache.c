#include "csapp.h"


void initCache(struct Cache *cache) {
    cache->all_size = 0;
    for (int i = 0; i < BUCKETNUM; ++i) {
        pthread_mutex_init(&cache->locks[i], NULL);
        cache->bucket[i].prev = cache->bucket[i].next = &cache->bucket[i];
    }
}

// simple string hash function.
uint64_t hashString(String* key) {
    uint64_t hash_value = 0;
    for (int i = 0; i < key->size_; ++i) {
        hash_value = hash_value * 13331 + key->data_[i];
    }
    return hash_value;
}

// 查找函数
String* findCache(struct Cache* cache, String* key) {
    size_t hashCode = hashString(key);
    size_t idx = hashCode % BUCKETNUM;
    pthread_mutex_lock(&cache->locks[idx]);
    struct CacheNode *find = NULL;
    for (struct CacheNode* cur = cache->bucket[idx].next; cur != &cache->bucket[idx]; cur = cur -> next) {
        if (cmpString(cur->key, key)) {
            find = cur;
            break;
        }
    }

    // 如果找到的话，就将自己放到当前桶的最后一个位置
    if (find) {
        find->next->prev = find->prev;
        find->prev->next = find->next;
        find->next = &cache->bucket[idx];
        find->prev = cache->bucket[idx].prev;
        find->next->prev = find;
        find->prev->next = find;
    }

    pthread_mutex_unlock(&cache->locks[idx]);
    return find ? find->val : NULL;
}

void putCache(struct Cache* cache, String *key, String *val) {
    size_t hashCode = hashString(key);
    size_t idx = hashCode % BUCKETNUM;
    if (val->size_ > MAX_OBJECT_SIZE) return;
    pthread_mutex_lock(&cache->locks[idx]);
    int ok = 0;
    if (cache->all_size + val->size_ > MAX_CACHE_SIZE) {
        for (int i = 1; i < BUCKETNUM / 2; ++i) {
            int new_idx = (idx + i) % BUCKETNUM;
            pthread_mutex_lock(&cache->locks[new_idx]);
            if (cache->bucket[new_idx].next != &cache->bucket[new_idx]) {
                struct CacheNode* to_delete = cache->bucket[new_idx].next;
                to_delete -> next -> prev = to_delete -> prev;
                to_delete -> prev -> next = to_delete -> next;
                cache->all_size -= to_delete->val->size_;
                ok = 1;
                freeString(to_delete->key);
                freeString(to_delete->val);
                free(to_delete);
                pthread_mutex_unlock(&cache->locks[new_idx]);
                break;
            }
            pthread_mutex_unlock(&cache->locks[new_idx]);
        }
    } else {
        ok = 1;
    }
    if (ok) {
        struct CacheNode *new_node = malloc(sizeof(*new_node));
        new_node->key = key;
        new_node->val = val;
        new_node->next = &cache->bucket[idx];
        new_node->prev = cache->bucket[idx].prev;
        new_node->next->prev = new_node;
        new_node->prev->next = new_node;
        cache->all_size += val->size_;
    }
    pthread_mutex_unlock(&cache->locks[idx]);
}