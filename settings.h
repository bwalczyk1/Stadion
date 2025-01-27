#define PLACES 3
#define CONTROLS_PER_PLACE 3

#define SHM_KEY_ID 'M'
#define SHM_INDEX_WAITING_NUMBER 0
#define SHM_SIZE_WITHOUT_PLACES 1

#define MSG_QUEUE_FAN 'F'
#define MSG_EMPTY_CONTROL 1
#define MSG_EMPTY_CONTROL_FOR_WAITING 2
#define MSG_QUEUE_FAN_TYPES 2
#define MSG_CONTROL_TAKEN -1
#define MSG_CONTROL_END -2

#define MSG_QUEUE_WORKER 'E'
#define MSG_FANS_LEFT 2

#define MSG_QUEUE_CONTROL 'C'
#define MSG_INITIATE_CONTROL 1
#define MSG_QUEUE_CONTROL_TYPES 1

#define MSG_QUEUE_BOSS 'B'
#define MSG_BOSS_INFO 1

#define SEM_KEY_ID 'S'
#define SEM_ENTRANCE_CONTROL 0
#define SEM_ENTRANCE 1
#define SEM_ENTRANCE_VIP 2
#define SEM_EXIT 3
#define SEM_EXIT_VIP 4
#define SEM_EXIT_CONTROL 5

#define SEM_NUMBER 6
#define TRIES 3

#define VIP_PER_MILLE 3
#define TEAM_1_CHANCE 50
#define CHILD_CHANCE 20
#define THREAT_CHANCE 30
#define CHILD_THREAT_CHANCE 5

#define FANS 400
#define K 100