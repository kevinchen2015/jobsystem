#pragma once

//对上层暴露的封装

//entyty type
#define FISH	1
#define BULLET	2

//component type
#define SPACE_COMPONENT		COMPONENT_ID_BEGIN + 1
#define COLLISION_COMPONENT COMPONENT_ID_BEGIN + 2

#define MAX_COMPNET_NUM 6

#define CMD_TICK 0
#define ADD_OBJ 1
#define REMOVE_OBJ 2
#define MODIFY_OBJ 3

#define SYNC_MOVE 4
#define SYNC_COLLID 5

struct GameCmd
{
	int cmd_id;
	union
	{
		struct Tick
		{
		}tick;
		struct AddObj
		{
			int id;
			int type;
			float x, y;
			float w, h;
			float speed;
			float angle;
		}add_obj;
		struct RemoveObj
		{
			int id;
		}remove_obj;
		struct ModifyObj
		{
			int id;
			float speed;
			float angle;
		}modify_obj;
		struct SyncMove
		{
			int id;
			float x, y;
		}sync_move;
		struct SyncCollider
		{
			int id;
			int hit_id;
		}sync_collider;
	};
};

typedef void(*OnRecvFunc)(GameCmd&);

void JobWorkerInit(int interval,int thread_num = 4 );

void JobWorkerUninit();

void JobWorkerUpdate(OnRecvFunc cb);

void JobCmdPushToGame(GameCmd& cmd);