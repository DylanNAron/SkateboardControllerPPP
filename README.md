# 🛹 SkateboardControllerPPP 🛹
Custom Skateboard Character in Unreal Engine for a Personal Programming Project. Extended a custom movement component off of Unreal's base character movement component. This skateboard controller includes realistic skate movement, a follow cam, flipping, spinning, lots of motion capture animations from me, analog flick gesture recognition, and grinding!

[Demo Video](https://youtu.be/kybtRQHAj00?si=zth4hKPkkCne1Pkr)

## Analog Flick Trick System
![Analog Flick Trick System in Action](https://github.com/DylanNAron/SkateboardControllerPPP/blob/main/SkateFlick.gif)

## Aerial Spinning/Flipping
![Aerial Spinning and Flipping Demonstration](https://github.com/DylanNAron/SkateboardControllerPPP/blob/main/SkateFlip.gif)

## Code Organization
SkateCharacter.cpp  
├── SkateCharacterMovementComponent.cpp  
└── AnalogStickTrickSystem.cpp

To easily add new skate flick tricks, tweak movement values, or even just update visuals, these can be done in the `BP_MySkateCharacter` Blueprint.

For detailed insights, check out my [Skateboard Controller PPP Blogs](https://dylannaron.github.io/#blogs).
