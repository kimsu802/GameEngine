#include "Input.h"
#include <cassert>
#include <windows.h>

namespace Craft
{
    // static 변수 초기화
    Input* Input::instance = nullptr;

    Input::Input()
    {
        //시작할 때 instance 값은 nullptr이어야 한다.
        assert(!instance && "instance should be null none !");

        instance = this;
    }

    bool Input::GetKeyDown(int keyCode) const
    {
        return !keyStates[keyCode].wasKeyDown && keyStates[keyCode].isKeyDown;
    }

    bool Input::GetKeyUp(int keyCode) const
    {
        return keyStates[keyCode].wasKeyDown && !keyStates[keyCode].isKeyDown;
    }

    bool Input::GetKey(int keyCode) const
    {
        return keyStates[keyCode].isKeyDown;
    }

    Input& Input::Get()
    {
        // instance는 null 이면 안됨.
        assert(instance && "instance should not be nullptr !");

        return *instance;
    }

    void Input::ProcessInput()
    {
        // 현재 프레임에 키 입력이 발생했는지 확인.
        for (int ix = 0; ix < keyCount; ++ix)
        {
            // 키 눌림 여부 저장.
            // 0x8000은 최상위 비트.
            keyStates[ix].isKeyDown = ((GetAsyncKeyState(ix) & 0x8000) != 0);
        }
    }

    void Input::SavePreviousStates()
    {
        // 이전 프레임 입력 값 저장
        for (KeyState& state : keyStates)
        {
            // 현재 프레임 입력 값을 이전 프레임 값으로 저장.
            state.wasKeyDown = state.isKeyDown;
        }
    }

}

