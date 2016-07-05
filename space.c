#include "space.h"

cpShapeFilter GRAB_FILTER = {CP_NO_GROUP, GRABBABLE_MASK_BIT, GRABBABLE_MASK_BIT};
cpShapeFilter NOT_GRABBABLE_FILTER = {CP_NO_GROUP, ~GRABBABLE_MASK_BIT, ~GRABBABLE_MASK_BIT};

static cpBody  *mouse_body       = NULL;
static cpConstraint *mouse_joint = NULL;
static cpVect mouse_pnt;

static void update_cursor();
static void freeSpaceChildren(cpSpace *space);

cpSpace *
space_init(int width, int height) {

  cpSpace *space = cpSpaceNew();
  cpSpaceSetIterations(space, 5);
  cpSpaceSetGravity(space, cpv(0, 100));
  cpSpaceSetSleepTimeThreshold(space, 0.5f);
  cpSpaceSetCollisionSlop(space, 0.5f);
  
  cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
  cpShape *shape;
  
  // Create segments around the edge of the screen.
  shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,-height), cpv(0,height), 0.0f));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 1.0f);
  cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

  shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(width,-height), cpv(width,height), 0.0f));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 1.0f);
  cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

  shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,height), cpv(width,height), 0.0f));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 1.0f);
  cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
  
  // Add lots of boxes.
  for(int i=0; i<12; i++){
    for(int j=0; j<=i; j++){

      float size = 20.0;
      body = cpSpaceAddBody(space, cpBodyNew(1.0f, cpMomentForBox(1.0f, size, size*1.618)));
      cpBodySetPosition(body, cpv(width/2+j*32 - i*16, i*size*2.0));
      
      shape = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size*1.618, 0.5f));
      cpShapeSetElasticity(shape, 0.0f);
      cpShapeSetFriction(shape, 0.8f);
    
    }
  }
  
  // Add a ball to make things more interesting
  cpFloat radius = 15.0f;
  body = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
  cpBodySetPosition(body, cpv(width/2.0, -height/2.0 + radius+5));

  shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
  cpShapeSetElasticity(shape, 0.0f);
  cpShapeSetFriction(shape, 0.9f);

  mouse_body = cpBodyNewKinematic();
  
  return space;
}

void
space_update(cpSpace *space, double dt) {
  update_cursor();
  cpSpaceStep(space, dt);
}

void
space_destroy(cpSpace *space) {
  freeSpaceChildren(space);
  cpSpaceFree(space);
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
static void
freeSpaceChildren(cpSpace *space) {
  // Must remove these BEFORE freeing the body or you will access dangling pointers.
  cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)PostShapeFree, space);
  cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)PostConstraintFree, space);
  cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)PostBodyFree, space);
}

// EVENTS
void
space_mouse_down(cpSpace* space) {

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

void
space_mouse_up(cpSpace* space) {
  if(mouse_joint){
    cpSpaceRemoveConstraint(space, mouse_joint);
    cpConstraintFree(mouse_joint);
    mouse_joint = NULL;
  }
}

void
space_mouse_move(cpSpace* space, int x, int y) {
  mouse_pnt.x = x;
  mouse_pnt.y = y;
}

static void 
update_cursor() {
  cpVect new_point = cpvlerp(mouse_body->p, mouse_pnt, 0.25f);
  mouse_body->v = cpvmult(cpvsub(new_point, mouse_body->p), 60.0f);
  mouse_body->p = new_point;
}
