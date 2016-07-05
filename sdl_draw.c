//SDL DRAW IMPLEMENTATION

static void
DrawCircle(
  cpVect  p, 
  cpFloat a, 
  cpFloat r, 
  cpSpaceDebugColor outline, 
  cpSpaceDebugColor fill, 
  cpDataPointer     data) {

  uint c = 
    ((uint)(fill.r * 255)<<24)|
    ((uint)(fill.g * 255)<<16)|
    ((uint)(fill.b * 255)<< 8)|0xFF;

  filledCircleColor(screen, p.x, p.y, r, c);  
  // circleColor(screen, p.x, p.y, r, c);  
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
    ((uint)(fill.r*255)<<24)|
    ((uint)(fill.g*255)<<16)|
    ((uint)(fill.b*255)<< 8)|0xFF;

  Sint16 vx[count], vy[count];
  for(int i=0; i<count; i++) {
    vx[i] = verts[i].x;
    vy[i] = verts[i].y;
  }
  filledPolygonColor(screen, vx, vy, count, c);
  // polygonColor(screen, vx, vy, count, c);
}

static void
DrawDot(
  cpFloat size, 
  cpVect p, 
  cpSpaceDebugColor color, 
  cpDataPointer data){

  uint c = 
    ((uint)(color.r*255)<<24)|
    ((uint)(color.g*255)<<16)|
    ((uint)(color.b*255)<< 8)|0xFF;

  // lineColor(screen, pos.x     , pos.y-size, pos.x     , pos.y+size, c);
  // lineColor(screen, pos.x-size, pos.y     , pos.x+size, pos.y     , c);
  circleColor(screen, p.x, p.y, 5, c);  

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
