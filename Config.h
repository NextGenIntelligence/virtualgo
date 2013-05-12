#ifndef CONFIG_H
#define CONFIG_H

#if defined( __APPLE__ ) && TARGET_OS_IPHONE
#define OPENGL_ES2 1
#endif

//#define USE_SECONDARY_DISPLAY_IF_EXISTS
//#define DEBUG_SHADOW_VOLUMES
//#define FRUSTUM_CULLING
//#define DISCOVER_KEY_CODES

const float SceneGridRes = 5;
const float SceneGridBounds = 64;

const float FatFingerBonus = 1.35f;

const int BoardSize = 9;

const int MaxStarPoints = 9;

const int MaxTouches = 64;

const float LockedDragSpeedMax = 10.0f;
const float FlickSpeedThreshold = 20.0f;

const float PlaceStoneHardThreshold = 0.075f;

const float ZoomIn = 25;
const float ZoomOut = 48;

const float ZoomInTightness = 0.25f;
const float ZoomOutTightness = 0.15f;

const float AccelerometerFrequency = 20;
const float AccelerometerTightness = 0.1f;

const float JerkThreshold = 0.1f;
const float JerkScale = 0.5f;

const float LaunchThreshold = 0.75f;
const float LaunchMomentum = 8;

const float MinimumSwipeLength = 50;            // points
const float SwipeLengthPerSecond = 250;         // points
const float MaxSwipeTime = 1.0f;                // seconds
const float SwipeMomentum = 10.0f;

const float HoldDelay = 0.05f;                  // seconds
const float HoldDamping = 0.75f;                
const float HoldMoveThreshold = 40;             // points

const float SelectDamping = 0.75f;

const float TouchImpulse = 1.0f;

#endif
