#ifndef _LINUX_PID_H
#define _LINUX_PID_H

enum pid_type
{
	PIDTYPE_PID,    // pid    线程 ID
	PIDTYPE_TGID,   // tgid   进程 ID
	PIDTYPE_PGID,   // pgid   进程组 ID
	PIDTYPE_SID,    // sid    会话 ID
	PIDTYPE_MAX     // 确定类型的长度(聪明)
};

/**
 * 用来快速索引 pid 的数据结构
 */
struct pid
{
	/* Try to keep pid_chain in the same cacheline as nr for find_pid */
	int nr;       // pid
	struct hlist_node pid_chain;  // 相同 hash 指的不同pid集合索引链表
	/* list of pids with the same nr, only one of them is in the hash */
	struct list_head pid_list;    // 位于同一个线程组的不同线程集合
};

/*
 * elem : &pid->pid_list
 * type : pid_type
 *
 * 最终根据 elem(task_struct->pids[type]->pid_list) - 结构体偏移量 计算出 task_struct 的首地址
 * 太牛逼了
 */
#define pid_task(elem, type) \
	list_entry(elem, struct task_struct, pids[type].pid_list)

/*
 * attach_pid() and detach_pid() must be called with the tasklist_lock
 * write-held.
 */
extern int FASTCALL(attach_pid(struct task_struct *task, enum pid_type type, int nr));

extern void FASTCALL(detach_pid(struct task_struct *task, enum pid_type));

/*
 * look up a PID in the hash table. Must be called with the tasklist_lock
 * held.
 */
extern struct pid *FASTCALL(find_pid(enum pid_type, int));

extern int alloc_pidmap(void);
extern void FASTCALL(free_pidmap(int));
extern void switch_exec_pids(struct task_struct *leader, struct task_struct *thread);

#define do_each_task_pid(who, type, task)				\
	if ((task = find_task_by_pid_type(type, who))) {		\
		prefetch((task)->pids[type].pid_list.next);		\
		do {

#define while_each_task_pid(who, type, task)				\
		} while (task = pid_task((task)->pids[type].pid_list.next,\
						type),			\
			prefetch((task)->pids[type].pid_list.next),	\
			hlist_unhashed(&(task)->pids[type].pid_chain));	\
	}								\

#endif /* _LINUX_PID_H */
