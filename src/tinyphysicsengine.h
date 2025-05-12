#ifndef _TINYPHYSICSENGINE_H
#define _TINYPHYSICSENGINE_H

#ifndef TPSH_H
#define TPSH_H

/**
  tinyphysicsengine (TPE)

  Simple/suckless header-only hybrid 3D physics engine with no floating point,
  only 32 bit int arithmetic, similar to e.g. small3dlib.
  
  Conventions and formats are the same or similar to those of small3dlib so as
  to make them easily integrate with each other.

  The library works with bodies made of spheres connected by elastic springs,
  i.e. soft bodies which however behave as "stiff" bodies by default and can
  be used to fake rigid body physics as well. Bodies are placed in environemnts
  specified by a distance function that allows to implement any mathematical
  shape.

  Orientations/rotations are in extrinsic Euler angles in the ZXY order (by Z,
  then by X, then by Y), if not mentioned otherwise. Angles are in TPE_Units,
  TPE_FRACTIONS_PER_UNIT is full angle (2 PI). Sometimes rotations can also be
  specified in the "about axis" format: here the object is rotated CW by given
  axis by an angle that's specified by the magnitude of the vector.

  Where it matters (e.g. rotations about axes) we consider a left-handed coord.
  system (x right, y up, z forward).

  ------------------------------------------------------------------------------

  by drummyfish, 2022

  version 0.8d

  This work's goal is to never be encumbered by any exclusive intellectual
  property rights. The work is therefore provided under CC0 1.0
  (https://creativecommons.org/publicdomain/zero/1.0/) + additional WAIVER OF
  ALL INTELLECTUAL PROPERTY RIGHTS that waives the rest of intellectual property
  rights not already waived by CC0 1.0. The WAIVER OF ALL INTELLECTUAL PROPERTY
  RGHTS is as follows:

  Each contributor to this work agrees that they waive any exclusive rights,
  including but not limited to copyright, patents, trademark, trade dress,
  industrial design, plant varieties and trade secrets, to any and all ideas,
  concepts, processes, discoveries, improvements and inventions conceived,
  discovered, made, designed, researched or developed by the contributor either
  solely or jointly with others, which relate to this work or result from this
  work. Should any waiver of such right be judged legally invalid or
  ineffective under applicable law, the contributor hereby grants to each
  affected person a royalty-free, non transferable, non sublicensable, non
  exclusive, irrevocable and unconditional license to this right. 
*/

#include <stdint.h>

typedef int32_t TPE_Unit;               ///< Basic fixed point unit type.
typedef int16_t TPE_UnitReduced;        ///< Like TPE_Unit but saving space

#define TPE_FRACTIONS_PER_UNIT 512      ///< one fixed point unit, don't change
#define TPE_F TPE_FRACTIONS_PER_UNIT    ///< short for TPE_FRACTIONS_PER_UNIT
#define TPE_JOINT_SIZE_MULTIPLIER 32    ///< joint size is scaled (size saving)

#define TPE_INFINITY 2147483647

#define TPE_JOINT_SIZE(joint) ((joint).sizeDivided * TPE_JOINT_SIZE_MULTIPLIER)

#ifndef TPE_APPROXIMATE_LENGTH
  #define TPE_APPROXIMATE_LENGTH 0      /**< whether or not use length/distance 
                                           approximation rather than exact 
                                           calculation (1 is faster but less
                                           accurate), beware of possible lower
                                           stability */
#endif

#if !TPE_APPROXIMATE_LENGTH
  #define TPE_DISTANCE TPE_dist
  #define TPE_LENGTH TPE_vec3Len
#else
  #define TPE_DISTANCE TPE_distApprox
  #define TPE_LENGTH TPE_vec3LenApprox
#endif

#ifndef TPE_LOG
  #define TPE_LOG(s) ; // redefine to some print function to show debug logs
#endif

#ifndef TPE_LOW_SPEED
/** Speed, in TPE_Units per ticks, that is considered low (used e.g. for auto
  deactivation of bodies). */
  #define TPE_LOW_SPEED 30
#endif

#ifndef TPE_RESHAPE_TENSION_LIMIT
/** Tension limit, in TPE_Units, after which a non-soft body will be reshaped.
  Smaller number will keep more stable shapes but will cost more performance. */
  #define TPE_RESHAPE_TENSION_LIMIT 20
#endif

#ifndef TPE_RESHAPE_ITERATIONS
/** How many iterations of reshaping will be performed by the step function if
  the body's shape needs to be reshaped. Greater number will keep shapes more
  stable but will cost some performance. */
  #define TPE_RESHAPE_ITERATIONS 3
#endif

#ifndef TPE_DEACTIVATE_AFTER
/** After how many ticks of low speed should a body be disabled. This mustn't
  be greater than 255. */
  #define TPE_DEACTIVATE_AFTER 128
#endif

#ifndef TPE_LIGHT_DEACTIVATION
/** When a body is activated by a collision, its deactivation counter will be
  set to this value, i.e. after a collision the body will be prone to deactivate
  sooner than normally. This is to handle situations with many bodies touching
  each other that would normally keep activating each other, never coming to
  rest. */
  #define TPE_LIGHT_DEACTIVATION \
    (TPE_DEACTIVATE_AFTER - TPE_DEACTIVATE_AFTER / 10)
#endif

#ifndef TPE_TENSION_ACCELERATION_DIVIDER
/** Number by which the base acceleration (TPE_FRACTIONS_PER_UNIT per tick
  squared) caused by the connection tension will be divided. This should be
  power of 2. */
  #define TPE_TENSION_ACCELERATION_DIVIDER 32
#endif

#ifndef TPE_TENSION_ACCELERATION_THRESHOLD
/** Limit within which acceleration caused by connection tension won't be
  applied. */
  #define TPE_TENSION_ACCELERATION_THRESHOLD 5
#endif

#ifndef TPE_TENSION_GREATER_ACCELERATION_THRESHOLD
/** Connection tension threshold after which twice as much acceleration will
  be applied. This helps prevent diverting joints that are "impaled" by
  environment.*/
  #define TPE_TENSION_GREATER_ACCELERATION_THRESHOLD \
    (TPE_TENSION_ACCELERATION_THRESHOLD * 3)
#endif

#ifndef TPE_COLLISION_RESOLUTION_ITERATIONS
/** Maximum number of iterations to try to uncollide two colliding bodies. */
  #define TPE_COLLISION_RESOLUTION_ITERATIONS 16
#endif

#ifndef TPE_COLLISION_RESOLUTION_MARGIN
/** Margin, in TPE_Units, by which a body will be shifted back to get out of
  collision. */
  #define TPE_COLLISION_RESOLUTION_MARGIN (TPE_F / 64)
#endif

#ifndef TPE_NONROTATING_COLLISION_RESOLVE_ATTEMPTS
/** Number of times a collision of nonrotating bodies with environment will be
  attempted to resolve. This probably won't have great performance implications
  as complex collisions of this kind should be relatively rare. */
  #define TPE_NONROTATING_COLLISION_RESOLVE_ATTEMPTS 8
#endif

#ifndef TPE_APPROXIMATE_NET_SPEED
/** Whether to use a fast approximation for calculating net speed of bodies
  which increases performance a bit. */
  #define TPE_APPROXIMATE_NET_SPEED 1
#endif

#define TPE_PRINTF_VEC3(v) printf("[%d %d %d]",(v).x,(v).y,(v).z);

typedef struct
{
  TPE_Unit x;
  TPE_Unit y;
  TPE_Unit z;
} TPE_Vec3;

/** Keeps given point within specified axis-aligned box. This can be used e.g.
  to smooth rendered movement of jittering physics bodies. */
TPE_Vec3 TPE_vec3KeepWithinBox(TPE_Vec3 point, TPE_Vec3 boxCenter,
  TPE_Vec3 boxMaxVect);

TPE_Vec3 TPE_vec3KeepWithinDistanceBand(TPE_Vec3 point, TPE_Vec3 center,
  TPE_Unit minDistance, TPE_Unit maxDistance);

TPE_Vec3 TPE_vec3(TPE_Unit x, TPE_Unit y, TPE_Unit z);
TPE_Vec3 TPE_vec3Minus(TPE_Vec3 v1, TPE_Vec3 v2);
TPE_Vec3 TPE_vec3Subtract(TPE_Vec3 v1, TPE_Vec3 v2);
TPE_Vec3 TPE_vec3Plus(TPE_Vec3 v1, TPE_Vec3 v2);
TPE_Vec3 TPE_vec3Cross(TPE_Vec3 v1, TPE_Vec3 v2);
TPE_Vec3 TPE_vec3Project(TPE_Vec3 v, TPE_Vec3 base);
TPE_Vec3 TPE_vec3ProjectNormalized(TPE_Vec3 v, TPE_Vec3 baseNormalized);
TPE_Vec3 TPE_vec3Times(TPE_Vec3 v, TPE_Unit units);
TPE_Vec3 TPE_vec3TimesPlain(TPE_Vec3 v, TPE_Unit q);
TPE_Vec3 TPE_vec3Normalized(TPE_Vec3 v);

TPE_Unit TPE_vec3Dot(TPE_Vec3 v1, TPE_Vec3 v2);
TPE_Unit TPE_vec3Len(TPE_Vec3 v);
TPE_Unit TPE_vec3LenApprox(TPE_Vec3 v);

/** Returns an angle in TPE_Units (see angle conventions) of a 2D vector with
  the X axis, CCW. */
TPE_Unit TPE_vec2Angle(TPE_Unit x, TPE_Unit y);

/** Keeps given value within specified range. This can be used e.g. for movement
  smoothing. */
TPE_Unit TPE_keepInRange(TPE_Unit x, TPE_Unit xMin, TPE_Unit xMax);

TPE_Unit TPE_abs(TPE_Unit x);
TPE_Unit TPE_max(TPE_Unit a, TPE_Unit b);
TPE_Unit TPE_min(TPE_Unit a, TPE_Unit b);
TPE_Unit TPE_nonZero(TPE_Unit x);
TPE_Unit TPE_dist(TPE_Vec3 p1, TPE_Vec3 p2);
TPE_Unit TPE_distApprox(TPE_Vec3 p1, TPE_Vec3 p2);
TPE_Unit TPE_sqrt(TPE_Unit x);

/** Compute sine, TPE_FRACTIONS_PER_UNIT as argument corresponds to 2 * PI
  radians. Returns a number from -TPE_FRACTIONS_PER_UNIT to
  TPE_FRACTIONS_PER_UNIT. */
TPE_Unit TPE_sin(TPE_Unit x);
TPE_Unit TPE_cos(TPE_Unit x);
TPE_Unit TPE_atan(TPE_Unit x);

typedef struct
{
  TPE_Vec3 position;
  TPE_UnitReduced velocity[3]; ///< not TPE_Vec3 to save size
  uint8_t sizeDivided; /**< size (radius, ...), for saving space divided by 
                            TPE_JOINT_SIZE_MULTIPLIER */
} TPE_Joint;

typedef struct
{
  uint8_t joint1;
  uint8_t joint2;
  uint16_t length;     ///< connection's preferred length, uint16_t saves space
} TPE_Connection;

#define TPE_BODY_FLAG_DEACTIVATED 1    /**< Not being updated due to low energy,
                                            "sleeping", will be woken by
                                            collisions etc. */
#define TPE_BODY_FLAG_NONROTATING 2    /**< When set, the body won't rotate, 
                                            will only move linearly. Here the
                                            velocity of the body's first joint
                                            is the velocity of the whole
                                            body. */
#define TPE_BODY_FLAG_DISABLED 4       /**< Disabled, not taking part in
                                            simulation. */
#define TPE_BODY_FLAG_SOFT 8           /**< Soft connections, effort won't be
                                            made to keep the body's shape. */
#define TPE_BODY_FLAG_SIMPLE_CONN 16   /**< Simple connections, don't zero out
                                            antagonist forces or apply
                                            connection friction, can increase
                                            performance. */
#define TPE_BODY_FLAG_ALWAYS_ACTIVE 32 /**< Will never deactivate due to low
                                            energy. */

/** Function used for defining static environment, working similarly to an SDF
  (signed distance function). The parameters are: 3D point P, max distance D.
  The function should behave like this: if P is inside the solid environment
  volume, P will be returned; otherwise closest point (by Euclidean distance) to
  the solid environment volume from P will be returned, except for a case when
  this closest point would be further away than D, in which case any arbitrary
  point further away than D may be returned (this allows for optimizations). */
typedef TPE_Vec3 (*TPE_ClosestPointFunction)(TPE_Vec3, TPE_Unit);

/** Function that can be used as a joint-joint or joint-environment collision
  callback, parameters are following: body1 index, joint1 index, body2 index,
  joint2 index, collision world position. If body1 index is the same as body1
  index, then collision type is body-environment, otherwise it is body-body
  type. The function has to return either 1 if the collision is to be allowed
  or 0 if it is to be discarded. This can besides others be used to disable
  collisions between some bodies. */
typedef uint8_t (*TPE_CollisionCallback)(uint16_t, uint16_t, uint16_t, uint16_t,
  TPE_Vec3);

/** Function used by the debug drawing functions to draw individual pixels to
  the screen. The parameters are following: pixel x, pixel y, pixel color. */
typedef void (*TPE_DebugDrawFunction)(uint16_t, uint16_t, uint8_t);

/** Physics body made of spheres (each of same weight but possibly different
  radia) connected by elastic springs. */
typedef struct
{
  TPE_Joint *joints;
  uint8_t jointCount;
  TPE_Connection *connections;
  uint8_t connectionCount;
  TPE_UnitReduced jointMass;       ///< mass of a single joint
  TPE_UnitReduced friction;        ///< friction of each joint
  TPE_UnitReduced elasticity;      ///< elasticity of each joint
  uint8_t flags;
  uint8_t deactivateCount;
} TPE_Body;

typedef struct
{
  TPE_Body *bodies;
  uint16_t bodyCount;
  TPE_ClosestPointFunction environmentFunction;
  TPE_CollisionCallback collisionCallback;
} TPE_World;

/** Tests the mathematical validity of given closest point function (function
  representing the physics environment), i.e. whether for example approaching
  some closest point in a straight line keeps approximately the same closest
  point. Note that this function may take a long time to complete, especially
  with higher gridResolution values and more complex environment functions. You
  should use this function to test your environment function, especially if you
  create functions for your own shapes etc. The cornerFrom and cornerTo points
  are corners of an axis-aligned box within which testing will take place, 
  gridResolution defines numbers of points (i.e. step length) along each
  dimension to test (recommended e.g. 64), allowedError says error within which
  points will be considered the same (recommended range approx. 10 to 200). If
  testing is successful, 1 is returned, otherwise 0 is returned and the point
  around which error was detected is returned in errorPoint (unless the pointer
  is 0 in which case it is ignored). */
uint8_t TPE_testClosestPointFunction(TPE_ClosestPointFunction f,
  TPE_Vec3 cornerFrom, TPE_Vec3 cornerTo, uint8_t gridResolution,
  TPE_UnitReduced allowedError, TPE_Vec3 *errorPoint);

void TPE_bodyInit(TPE_Body *body, 
  TPE_Joint *joints, uint8_t jointCount, 
  TPE_Connection *connections, uint8_t connectionCount,
  TPE_Unit mass);

void TPE_worldInit(TPE_World *world,
  TPE_Body *bodies, uint16_t bodyCount,
  TPE_ClosestPointFunction environmentFunction);

/** Gets orientation (rotation) of a body from a position of three of its
  joints. The vector from joint1 to joint2 is considered the body's forward
  direction, the vector from joint1 to joint3 its right direction. The returned
  rotation is in Euler angles (see rotation conventions). */
TPE_Vec3 TPE_bodyGetRotation(const TPE_Body *body, uint16_t joint1, 
  uint16_t joint2, uint16_t joint3);

void TPE_vec3Normalize(TPE_Vec3 *v);

/** Rotates a 3D point by given Euler angle rotation (see rotation
  conventions). */
TPE_Vec3 TPE_pointRotate(TPE_Vec3 point, TPE_Vec3 rotation);

/** Returns an inverse rotation to given rotation, in Euler angles (see rotation
  conventions). */
TPE_Vec3 TPE_rotationInverse(TPE_Vec3 rotation);

/** Returns a connection tension, i.e. a signed percentage difference against
  desired length (TPE_FRACTIONS_PER_UNIT means 100%). */
static TPE_Unit TPE_connectionTension(TPE_Unit length,
  TPE_Unit desiredLength);

/** Rotates a rotation specified in Euler angles by given axis + angle (see
  rotation conventions). Returns a rotation in Eurler angles. */
TPE_Vec3 TPE_rotationRotateByAxis(TPE_Vec3 rotation, TPE_Vec3 rotationByAxis);

/** Computes the formula of a 1D collision of rigid bodies. */
void TPE_getVelocitiesAfterCollision(TPE_Unit *v1, TPE_Unit *v2, TPE_Unit m1,
  TPE_Unit m2, TPE_Unit elasticity);

/** Computes orientation/rotation (see docs for orientation format) from two
  vectors (which should be at least close to being perpendicular and do NOT
  need to be normalized). This can be used to determine orientation of a body
  from a relative position of its joints. */
TPE_Vec3 TPE_rotationFromVecs(TPE_Vec3 forward, TPE_Vec3 right);

TPE_Joint TPE_joint(TPE_Vec3 position, TPE_Unit size);

/** Mostly for internal use, resolves a potential collision of two joints in a
  way that keeps the joints outside provided environment (if the function
  pointer is not 0). Returns 1 if joints collided or 0 otherwise. */
uint8_t TPE_jointsResolveCollision(TPE_Joint *j1, TPE_Joint *j2,
  TPE_Unit mass1, TPE_Unit mass2, TPE_Unit elasticity, TPE_Unit friction,
  TPE_ClosestPointFunction env);

/** Mostly for internal use, tests and potentially resolves a collision between
  a joint and environment, returns 0 if no collision happened, 1 if it happened
  and was resolved normally and 2 if it couldn't be resolved normally. */
uint8_t TPE_jointEnvironmentResolveCollision(TPE_Joint *joint, TPE_Unit
  elasticity, TPE_Unit friction, TPE_ClosestPointFunction env);

/** Tests whether a body is currently colliding with the environment. */
uint8_t TPE_bodyEnvironmentCollide(const TPE_Body *body,
  TPE_ClosestPointFunction env);

/** Mostly for internal use, tests and potentially resolves a collision of a
  body with the environment, returns 1 if collision happened or 0 otherwise. */
uint8_t TPE_bodyEnvironmentResolveCollision(TPE_Body *body, 
  TPE_ClosestPointFunction env);

TPE_Vec3 TPE_bodyGetLinearVelocity(const TPE_Body *body);

/** Computes the minimum bounding box of given body. */
void TPE_bodyGetAABB(const TPE_Body *body, TPE_Vec3 *vMin, TPE_Vec3 *vMax);

/** Computes a bounding sphere of a body which is not minimal but faster to
  compute than the minimum bounding sphere. */
void TPE_bodyGetFastBSphere(const TPE_Body *body, TPE_Vec3 *center,
  TPE_Unit *radius);

/** Computes the minimum bounding sphere of a body (there is another function
  for a faster approximate bounding sphere). */
void TPE_bodyGetBSphere(const TPE_Body *body, TPE_Vec3 *center,
  TPE_Unit *radius);

uint8_t TPE_checkOverlapAABB(TPE_Vec3 v1Min, TPE_Vec3 v1Max, TPE_Vec3 v2Min,
  TPE_Vec3 v2Max);

/** Mostly for internal use, checks and potentiall resolves collision of two
  bodies so as to keep them outside given environment. Returns 1 if collision
  happened or 0 otherwise. */
uint8_t TPE_bodiesResolveCollision(TPE_Body *b1, TPE_Body *b2,
  TPE_ClosestPointFunction env);

/** Pins a joint of a body to specified location in space (sets its location
  and zeros its velocity). */
void TPE_jointPin(TPE_Joint *joint, TPE_Vec3 position);

/** "Fakes" a rotation of a moving sphere by rotating it in the direction of
  its movement; this can create the illusion of the sphere actually rotating
  due to friction even if the physics sphere object (a body with a single joint)
  isn't rotating at all. Returns a rotation in the "about axis" format (see
  library conventions). */
TPE_Vec3 TPE_fakeSphereRotation(TPE_Vec3 position1, TPE_Vec3 position2,
  TPE_Unit radius);

/** Casts a ray against environment and returns the closest hit of a surface. If
  no surface was hit, a vector with all elements equal to TPE_INFINITY will be
  returned. The function internally works differently for outside rays (rays
  cast from the outside of the environment) and inside rays. Outside rays can
  be traced with raymarching and will be processed very quickly and precisely;
  in this case if any intersection is found, the function will try to return a
  point outside (not guaranteed) the environment that's just in front of the hit
  surface. Inside rays are difficult and slow to trace because environment
  function won't provide distance, so the results aren't guaranteed to be
  precise (the ray may miss some intersections); here rays will be traced by
  given step (insideStepSize) and eventually iterated a bit towards the
  intersection -- if any intersection is found, the function will try to return
  a point inside (not guaranteed) the environment just before the hit
  surface. */
TPE_Vec3 TPE_castEnvironmentRay(TPE_Vec3 rayPos, TPE_Vec3 rayDir,
  TPE_ClosestPointFunction environment, TPE_Unit insideStepSize,
  TPE_Unit rayMarchMaxStep, uint32_t maxSteps);

/** Casts a ray against bodies in a world (ignoring the environment), returns
  the position of the closest hit as well as the hit body's index in bodyIndex
  (unless the bodyIndex pointer is 0 in which case it is ignored). Similarly
  with jointIndex. If no hit is found a vector with all elements equal to
  TPE_INFINITY will be returned and bodyIndex will be -1. A specific body can be
  excluded with excludeBody (negative value will just make this parameter
  ignored). */
TPE_Vec3 TPE_castBodyRay(TPE_Vec3 rayPos, TPE_Vec3 rayDir, int16_t excludeBody,
  const TPE_World *world, int16_t *bodyIndex, int16_t *jointIndex);

/** Performs one step (tick, frame, ...) of the physics world simulation
  including updating positions and velocities of bodies, collision detection and
  resolution, possible reshaping or deactivation of inactive bodies etc. The
  time length of the step is relative to all other units but it's ideal if it is
  1/30th of a second. */
void TPE_worldStep(TPE_World *world);

void TPE_worldDeactivateAll(TPE_World *world);
void TPE_worldActivateAll(TPE_World *world);

TPE_Unit TPE_worldGetNetSpeed(const TPE_World *world);
TPE_Unit TPE_bodyGetNetSpeed(const TPE_Body *body);
TPE_Unit TPE_bodyGetAverageSpeed(const TPE_Body *body);
void TPE_bodyMultiplyNetSpeed(TPE_Body *body, TPE_Unit factor);
void TPE_bodyLimitAverageSpeed(TPE_Body *body, TPE_Unit speedMin,
  TPE_Unit speedMax);

/** Deactivates a body (puts it to sleep until another collision or force wake
  up). */
void TPE_bodyDeactivate(TPE_Body *body);

uint8_t TPE_bodyIsActive(const TPE_Body *body);

/** Attempts to shift the joints of a soft body so that the tension of all
  springs becomes zero while keeping the joints near their current position.
  This function performs one iteration of the equalizing algorithm and doesn't
  guarantee a perfect solution, it may help to run multiple iterations (call
  this function multiple times). */
void TPE_bodyReshape(TPE_Body *body, TPE_ClosestPointFunction
  environmentFunction);

/** Mostly for internal use, performs some "magic" on body connections, mainly
  cancelling out of velocities going against each other and also applying
  connection friction in soft bodies. The strong parameter indicates if the
  body is soft or not. */
void TPE_bodyCancelOutVelocities(TPE_Body *body, uint8_t strong);

/** Moves a body by certain offset. */
void TPE_bodyMoveBy(TPE_Body *body, TPE_Vec3 offset);

/** Moves a body (its center of mass) to given position. */
void TPE_bodyMoveTo(TPE_Body *body, TPE_Vec3 position);

/** Zeros velocities of all soft body joints. */
void TPE_bodyStop(TPE_Body *body);

void TPE_bodyActivate(TPE_Body *body);

/** Adds velocity to a soft body. */
void TPE_bodyAccelerate(TPE_Body *body, TPE_Vec3 velocity);

void TPE_bodyApplyGravity(TPE_Body *body, TPE_Unit downwardsAccel);

/** Adds angular velocity to a soft body. The rotation vector specifies the axis
  of rotation by its direction and angular velocity by its magnitude (magnitude
  of TPE_FRACTIONS_PER_UNIT will add linear velocity of TPE_FRACTIONS_PER_UNIT
  per tick to a point in the distance of TPE_FRACTIONS_PER_UNIT from the
  rotation axis). */
void TPE_bodySpin(TPE_Body *body, TPE_Vec3 rotation);

/** Same as TPE_bodySpin but additionally allows to specify the center of
  the spin. */
void TPE_bodySpinWithCenter(TPE_Body *body, TPE_Vec3 rotation, TPE_Vec3 center);

/** Instantly rotates a body about an axis (see library conventions for
  the rotation format). */
void TPE_bodyRotateByAxis(TPE_Body *body, TPE_Vec3 rotation);

/** Computes the center of mass of a body. This averages the position of all
  joints; note that if you need, you may estimate the center of the body faster,
  e.g. by taking a position of a single "center joint", or averaging just 2
  extreme points. */ 
TPE_Vec3 TPE_bodyGetCenterOfMass(const TPE_Body *body);

/** Draws a debug view of a 3D physics world using a provided pixel drawing
  function. This can be used to overlay a simple visualization of the physics
  objects to your main render, to spot exact borders of objects etc. The
  function draws simple dotted lines and circles with different "colors" for
  different types of objects (joints, connections, environemnt). camPos, camRot
  and camView should match the camera settings of your main renderer. CamView.x
  is horizontal resolution in pixels, camView.y is the vertical resolution,
  CamView.z says the camera focal length (~FOV) in TPE_Units (0 means
  orthographic projection). envGridRes is the resolution of an environment probe
  grid (the function will probe points in space and draw borders of the physics
  environemnt), envGridSize is the size (int TPE_Units) of the grid cell. Note
  the function may be slow (reducing envGridRes can help, workable value can be
  e.g. 16). */
void TPE_worldDebugDraw(TPE_World *world, TPE_DebugDrawFunction drawFunc,
  TPE_Vec3 camPos, TPE_Vec3 camRot, TPE_Vec3 camView, uint16_t envGridRes,
  TPE_Unit envGridSize);

#define TPE_DEBUG_COLOR_CONNECTION 0
#define TPE_DEBUG_COLOR_JOINT 1
#define TPE_DEBUG_COLOR_ENVIRONMENT 2
#define TPE_DEBUG_COLOR_INACTIVE 3

uint32_t TPE_jointHash(const TPE_Joint *joint);
uint32_t TPE_connectionHash(const TPE_Connection *connection);
uint32_t TPE_bodyHash(const TPE_Body *body);

/** Computes 32 bit hash of the world, useful for checking if two states of the
  world differ. The function takes into account most of the relevant state but
  possibly not all of it, for details check the code. */
uint32_t TPE_worldHash(const TPE_World *world);

// FUNCTIONS FOR GENERATING BODIES

void TPE_makeBox(TPE_Joint joints[8], TPE_Connection connections[16],
  TPE_Unit width, TPE_Unit depth, TPE_Unit height, TPE_Unit jointSize);
void TPE_makeCenterBox(TPE_Joint joints[9], TPE_Connection connections[18],
  TPE_Unit width, TPE_Unit depth, TPE_Unit height, TPE_Unit jointSize);
void TPE_makeRect(TPE_Joint joints[4], TPE_Connection connections[6],
  TPE_Unit width, TPE_Unit depth, TPE_Unit jointSize);
void TPE_makeTriangle(TPE_Joint joints[3], TPE_Connection connections[3],
  TPE_Unit sideLength, TPE_Unit jointSize);
void TPE_makeCenterRect(TPE_Joint joints[5], TPE_Connection connections[8],
  TPE_Unit width, TPE_Unit depth, TPE_Unit jointSize);
void TPE_makeCenterRectFull(TPE_Joint joints[5], TPE_Connection connections[10],
  TPE_Unit width, TPE_Unit depth, TPE_Unit jointSize);
void TPE_make2Line(TPE_Joint joints[2], TPE_Connection connections[1],
  TPE_Unit length, TPE_Unit jointSize);

// FUNCTIONS FOR BUILDING ENVIRONMENT

TPE_Vec3 TPE_envAABoxInside(TPE_Vec3 point, TPE_Vec3 center, TPE_Vec3 size);
TPE_Vec3 TPE_envAABox(TPE_Vec3 point, TPE_Vec3 center, TPE_Vec3 maxCornerVec);
TPE_Vec3 TPE_envBox(TPE_Vec3 point, TPE_Vec3 center, TPE_Vec3 maxCornerVec,
  TPE_Vec3 rotation);
TPE_Vec3 TPE_envSphere(TPE_Vec3 point, TPE_Vec3 center, TPE_Unit radius);
TPE_Vec3 TPE_envSphereInside(TPE_Vec3 point, TPE_Vec3 center, TPE_Unit radius);
TPE_Vec3 TPE_envHalfPlane(TPE_Vec3 point, TPE_Vec3 center, TPE_Vec3 normal);
TPE_Vec3 TPE_envGround(TPE_Vec3 point, TPE_Unit height);
TPE_Vec3 TPE_envInfiniteCylinder(TPE_Vec3 point, TPE_Vec3 center, TPE_Vec3
  direction, TPE_Unit radius);
TPE_Vec3 TPE_envCylinder(TPE_Vec3 point, TPE_Vec3 center, TPE_Vec3 direction,
  TPE_Unit radius);
TPE_Vec3 TPE_envCone(TPE_Vec3 point, TPE_Vec3 center, TPE_Vec3 direction,
  TPE_Unit radius);
TPE_Vec3 TPE_envLineSegment(TPE_Vec3 point, TPE_Vec3 a, TPE_Vec3 b);
TPE_Vec3 TPE_envHeightmap(TPE_Vec3 point, TPE_Vec3 center, TPE_Unit gridSize,
  TPE_Unit (*heightFunction)(int32_t x, int32_t y), TPE_Unit maxDist);

/** Environment function for triagnular prism, e.g. for ramps. The sides array
  contains three 2D coordinates of points of the triangle in given plane with
  respect to the center. WARNING: the points must be specified in counter
  clowckwise direction! The direction var specified axis direction (0, 1 or
  2).*/
TPE_Vec3 TPE_envAATriPrism(TPE_Vec3 point, TPE_Vec3 center,
  const TPE_Unit sides[6], TPE_Unit depth, uint8_t direction);

/* The following are helper macros for creating a union of shapes inside an
  environment function and accelerating them with bounding volumes. */

#define TPE_ENV_START(test,point) TPE_Vec3 _pBest = test, _pTest; \
  TPE_Unit _dBest = TPE_DISTANCE(_pBest,point), _dTest; \
  (void)(_pBest); (void)(_dBest); (void)(_dTest); (void)(_pTest); // supress war

#define TPE_ENV_NEXT(test,point) \
 { if (_pBest.x == point.x && _pBest.y == point.y && _pBest.z == point.z) \
     return _pBest; \
  _pTest = test; _dTest = TPE_DISTANCE(_pTest,point); \
  if (_dTest < _dBest) { _pBest = _pTest; _dBest = _dTest; } }

#define TPE_ENV_END return _pBest;

#define TPE_ENV_BCUBE_TEST(bodyBCubeC,bodyBCubeR,envBCubeC,envBCubeR) ( \
  (TPE_abs(envBCubeC.x - bodyBCubeC.x) <= ((bodyBCubeR) + (envBCubeR))) && \
  (TPE_abs(envBCubeC.y - bodyBCubeC.y) <= ((bodyBCubeR) + (envBCubeR))) && \
  (TPE_abs(envBCubeC.z - bodyBCubeC.z) <= ((bodyBCubeR) + (envBCubeR))))

#define TPE_ENV_BSPHERE_TEST(bodyBSphereC,bodyBSphereR,envBSphereC,envBSphereR)\
  (TPE_DISTANCE(bodyBSphereC,envBSphereC) <= ((bodyBSphereR) + (envBSphereR)))


#endif

#endif // guard
