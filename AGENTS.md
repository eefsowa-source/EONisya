# EONisya / EON MOTIF6

- `CMakeLists.txt`가 JUCE 버전·플러그인 형식·빌드 설정의 기준이다. 구현은 `Source/PluginProcessor.cpp`와 `Source/PluginEditor.cpp`에 있다.
- CMake는 `JUCE_DIR` 또는 로컬 JUCE를 우선 사용하고 없으면 JUCE를 가져온다. 설정 전에 실제 JUCE 경로와 빌드 디렉터리를 확인한다.
- 저장소에 자동 테스트 대상이 없으므로 빌드 통과를 신스 동작의 증거로 쓰지 않는다. MIDI 렌더, AU/VST3 검사, DAW 로드와 UI는 각각 확인한다.
