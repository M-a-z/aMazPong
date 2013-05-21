
#ifndef MAZPONG_MOVABLE_ITEM
#define MAZPONG_MOVABLE_ITEM

struct movable_item;

typedef int (*set_positionF)(struct movable_item *_this,float x,float y,float z);
typedef int (*set_velocityF)(struct movable_item *_this,float x,float y,float z);
typedef float (*get_coordF)(struct movable_item *);

typedef struct movable_item
{
    get_coordF getX;
    get_coordF getY;
    set_positionF set_position;
    set_velocityF set_velocity;
    float xpos;
    float ypos;
    float zpos; //maybe we wan't the text to fade to distance
    float speedx;
    float speedy;
    float speedz; //maybe we wan't the text to fade to distance
}movable_item;
    
int init_movable(movable_item *_this);

#endif
