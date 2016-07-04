#include <stdio.h>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include <chipmunk/chipmunk_private.h>
#include <chipmunk/chipmunk.h>

#define SCREEN_W  480
#define SCREEN_H  480

#define GRABBABLE_MASK_BIT (1<<31)
cpShapeFilter GRAB_FILTER = {CP_NO_GROUP, GRABBABLE_MASK_BIT, GRABBABLE_MASK_BIT};
cpShapeFilter NOT_GRABBABLE_FILTER = {CP_NO_GROUP, ~GRABBABLE_MASK_BIT, ~GRABBABLE_MASK_BIT};

static cpSpace *space            = NULL;
static cpBody *mouse_body        = NULL;
static cpConstraint *mouse_joint = NULL;
static cpVect mouse_pnt;

static inline cpSpaceDebugColor RGBAColor(float r, float g, float b, float a){
  cpSpaceDebugColor color = {r, g, b, a};
  return color;
}

static inline cpSpaceDebugColor LAColor(float l, float a){
  cpSpaceDebugColor color = {l, l, l, a};
  return color;
}

static cpSpace * init(void);
static void      update(cpSpace *space, double dt);
static void      destroy(cpSpace *space);
static void      MouseMove(SDL_Event* evt);
static void      MouseDown(SDL_Event* evt);
static void      MouseUp(SDL_Event* evt);
static void      DrawImpl(cpSpace *space);
static void      Cursor();

static SDL_Surface *screen = 0;

int main(void){
  
  space = init();

  SDL_Event evt; 

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    return -1;
  }

  screen = SDL_SetVideoMode(
    SCREEN_W, 
    SCREEN_H, 
    24, SDL_HWSURFACE | SDL_DOUBLEBUF);

  if (screen == NULL) {
    return -1;
  }

  while(1) {
    while(SDL_PollEvent(&evt)) {
      if(evt.type == SDL_QUIT) {
        goto finish;
      }
      if (evt.type == SDL_KEYUP && evt.key.keysym.sym == SDLK_ESCAPE) {
        goto finish;
      }

      if (evt.type == SDL_MOUSEMOTION    ) MouseMove(&evt);
      if (evt.type == SDL_MOUSEBUTTONDOWN) MouseDown(&evt);
      if (evt.type == SDL_MOUSEBUTTONUP  ) MouseUp  (&evt);
    }

    Cursor();

    SDL_LockSurface(screen);

    SDL_FillRect(screen, NULL, 0x000080); 

    update(space, 0.01);
    DrawImpl(space);

    SDL_FreeSurface(screen);
    SDL_Flip(screen);
  }
  
finish:
  
  destroy(space);  
  SDL_FreeSurface(screen);
  SDL_Quit();

  return 0;
}

static void 
Cursor() {
  cpVect new_point = cpvlerp(mouse_body->p, mouse_pnt, 0.25f);
  mouse_body->v = cpvmult(cpvsub(new_point, mouse_body->p), 60.0f);
  mouse_body->p = new_point;
}

static cpSpace *
init(void) {

  cpSpace *space = cpSpaceNew();
  cpSpaceSetIterations(space, 30);
  cpSpaceSetGravity(space, cpv(0, 100));
  cpSpaceSetSleepTimeThreshold(space, 0.5f);
  cpSpaceSetCollisionSlop(space, 0.5f);
  
  cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
  cpShape *shape;
  
  // Create segments around the edge of the screen.
  shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,0), cpv(0,SCREEN_H), 0.0f));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 1.0f);
  cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

  shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(SCREEN_W,0), cpv(SCREEN_W,SCREEN_H), 0.0f));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 1.0f);
  cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

  shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,SCREEN_H), cpv(SCREEN_W,SCREEN_H), 0.0f));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 1.0f);
  cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
  
  // Add lots of boxes.
  for(int i=0; i<12; i++){
    for(int j=0; j<=i; j++){

      float size = 20.0;
      body = cpSpaceAddBody(space, cpBodyNew(1.0f, cpMomentForBox(1.0f, size, size*1.618)));
      cpBodySetPosition(body, cpv(SCREEN_W/2+j*32 - i*16, i*size*2.0));
      
      shape = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size*1.618, 0.5f));
      cpShapeSetElasticity(shape, 0.0f);
      cpShapeSetFriction(shape, 0.8f);
    
    }
  }
  
  // Add a ball to make things more interesting
  cpFloat radius = 15.0f;
  body = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
  cpBodySetPosition(body, cpv(0, 0 + radius+5));

  shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
  cpShapeSetElasticity(shape, 0.0f);
  cpShapeSetFriction(shape, 0.9f);

  mouse_body = cpBodyNewKinematic();
  
  return space;
}

static void
update(cpSpace *space, double dt) {
  cpSpaceStep(space, dt);
}

static void 
ShapeFreeWrap(cpSpace *space, cpShape *shape, void *unused){
  cpSpaceRemoveShape(space, shape);
  cpShapeFree(shape);
}

static void 
PostShapeFree(cpShape *shape, cpSpace *space){
  cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ShapeFreeWrap, shape, NULL);
}

static void 
ConstraintFreeWrap(cpSpace *space, cpConstraint *constraint, void *unused){
  cpSpaceRemoveConstraint(space, constraint);
  cpConstraintFree(constraint);
}

static void 
PostConstraintFree(cpConstraint *constraint, cpSpace *space){
  cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ConstraintFreeWrap, constraint, NULL);
}

static void 
BodyFreeWrap(cpSpace *space, cpBody *body, void *unused){
  cpSpaceRemoveBody(space, body);
  cpBodyFree(body);
}

static void 
PostBodyFree(cpBody *body, cpSpace *space){
  cpSpaceAddPostStepCallback(space, (cpPostStepFunc)BodyFreeWrap, body, NULL);
}

// Safe and future proof way to remove and free all objects that have been added to the space.
void
freeSpaceChildren(cpSpace *space) {
  // Must remove these BEFORE freeing the body or you will access dangling pointers.
  cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)PostShapeFree, space);
  cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)PostConstraintFree, space);
  
  cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)PostBodyFree, space);
}

static void
destroy(cpSpace *space) {
  freeSpaceChildren(space);
  cpSpaceFree(space);
}

// DRAW

static void
DrawCircle(
  cpVect  p, 
  cpFloat a, 
  cpFloat r, 
  cpSpaceDebugColor outline, 
  cpSpaceDebugColor fill, 
  cpDataPointer     data) {

  // printf("ChipmunkDebugDrawCircle(p, a, r, outline, fill)\n");
}

static void
DrawSegment(
  cpVect a, 
  cpVect b, 
  cpSpaceDebugColor color, 
  cpDataPointer     data) {

  uint c = 
    ((uint)(color.r*255)<<24)|
    ((uint)(color.g*255)<<16)|
    ((uint)(color.b*255)<< 8)|0xFF;

  lineColor(screen, a.x, a.y, b.x, b.y, c);

  // printf("ChipmunkDebugDrawSegment(a, b, color)\n");
}

static void
DrawFatSegment(
  cpVect a, 
  cpVect b, 
  cpFloat r, 
  cpSpaceDebugColor outline, 
  cpSpaceDebugColor fill, 
  cpDataPointer     data) {
  
  lineColor(screen, a.x, a.y, b.x, b.y, 0xFFFFFFFF);

  // printf("ChipmunkDebugDrawFatSegment(a, b, r, outline, fill)\n");
}

static void
DrawPolygon(
  int count, 
  const cpVect *verts, 
  cpFloat r, 
  cpSpaceDebugColor outline, 
  cpSpaceDebugColor fill, 
  cpDataPointer     data){ 
  uint c = 
    ((uint)(outline.r*255)<<24)|
    ((uint)(outline.g*255)<<16)|
    ((uint)(outline.b*255)<< 8)|0xFF;

  for(int i=0; i<count-1; i++)
    lineColor(screen, verts[i].x, verts[i].y, verts[i+1].x, verts[i+1].y, c);  
  lineColor(screen, verts[count-1].x, verts[count-1].y, verts[0].x, verts[0].y, c);  
  // printf("ChipmunkDebugDrawPolygon(count, verts, r, outline, fill)\n");
}

static void
DrawDot(
  cpFloat size, 
  cpVect pos, 
  cpSpaceDebugColor color, 
  cpDataPointer data){

  uint c = 
    ((uint)(color.r*255)<<24)|
    ((uint)(color.g*255)<<16)|
    ((uint)(color.b*255)<< 8)|0xFF;

  lineColor(screen, pos.x     , pos.y-size, pos.x     , pos.y+size, c);
  lineColor(screen, pos.x-size, pos.y     , pos.x+size, pos.y     , c);

  // printf("ChipmunkDebugDrawDot(size, pos, color)\n");
}

static cpSpaceDebugColor
ColorForShape(cpShape *shape, cpDataPointer data) {
  if(cpShapeGetSensor(shape)){
    return LAColor(1.0f, 0.1f);
  } else {
    cpBody *body = cpShapeGetBody(shape);
    
    if(cpBodyIsSleeping(body)){
      return LAColor(0.2f, 1.0f);
    } else if(body->sleeping.idleTime > shape->space->sleepTimeThreshold) {
      return LAColor(0.66f, 1.0f);
    } else {
      uint32_t val = (uint32_t)shape->hashid;
      
      // scramble the bits up using Robert Jenkins' 32 bit integer hash function
      val = (val+0x7ed55d16) + (val<<12);
      val = (val^0xc761c23c) ^ (val>>19);
      val = (val+0x165667b1) + (val<<5);
      val = (val+0xd3a2646c) ^ (val<<9);
      val = (val+0xfd7046c5) + (val<<3);
      val = (val^0xb55a4f09) ^ (val>>16);
      
      float r = (float)((val>>0) & 0xFF);
      float g = (float)((val>>8) & 0xFF);
      float b = (float)((val>>16) & 0xFF);
      
      float max = (float)cpfmax(cpfmax(r, g), b);
      float min = (float)cpfmin(cpfmin(r, g), b);
      float intensity = (cpBodyGetType(body) == CP_BODY_TYPE_STATIC ? 0.15f : 0.75f);
      
      // Saturate and scale the color
      if(min == max){
        return RGBAColor(intensity, 0.0f, 0.0f, 1.0f);
      } else {
        float coef = (float)intensity/(max - min);
        return RGBAColor(
          (r - min)*coef,
          (g - min)*coef,
          (b - min)*coef,
          1.0f
        );
      }
    }
  }
}

static void
DrawImpl(cpSpace *space) {

  cpSpaceDebugDrawOptions drawOptions = {
    DrawCircle,
    DrawSegment,
    DrawFatSegment,
    DrawPolygon,
    DrawDot,
    
    (cpSpaceDebugDrawFlags)(CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_CONSTRAINTS | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS),
    
    {200.0f/255.0f, 210.0f/255.0f, 230.0f/255.0f, 1.0f},
    ColorForShape,
    {0.0f, 0.75f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    NULL,
  };
  
  cpSpaceDebugDraw(space, &drawOptions);
}

static void
MouseDown(SDL_Event* evt) {
  
  // SDL_MouseButtonEvent button = evt->button;

  // give the mouse click a little radius to make it easier to click small shapes.
  cpFloat radius = 5.0;
  
  cpPointQueryInfo info = {};
  cpShape *shape = cpSpacePointQueryNearest(space, mouse_pnt, radius, GRAB_FILTER, &info);
  
  if(shape && cpBodyGetMass(cpShapeGetBody(shape)) < INFINITY){
    // Use the closest point on the surface if the click is outside of the shape.
    cpVect nearest = (info.distance > 0.0f ? info.point : mouse_pnt);
    
    cpBody *body = cpShapeGetBody(shape);
    mouse_joint = cpPivotJointNew2(mouse_body, body, cpvzero, cpBodyWorldToLocal(body, nearest));
    mouse_joint->maxForce = 50000.0f;
    mouse_joint->errorBias = cpfpow(1.0f - 0.15f, 60.0f);
    cpSpaceAddConstraint(space, mouse_joint);
  }
}

static void
MouseUp(SDL_Event* evt) {
  if(mouse_joint){
    cpSpaceRemoveConstraint(space, mouse_joint);
    cpConstraintFree(mouse_joint);
    mouse_joint = NULL;
  }
}

static void
MouseMove(SDL_Event* evt) {
    SDL_MouseMotionEvent motion = evt->motion;
    mouse_pnt.x = motion.x;
    mouse_pnt.y = motion.y;
}
