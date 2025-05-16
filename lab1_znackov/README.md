# Guide
В данной папке находятся ещё 2 папки. **external clang plugin** - версия плагина, которая билдится отдельно от **llvm** и вторая версия, на случай, если с этой возникнут проблемы, - билдится вместе.
Для метасборки первой необходимо написать: 
```
cmake -G Ninja \
  -DLLVM_DIR=путь до llvm(Например, /Users/shpkkk/compiler-project/tttc_practice/build_rel/lib/cmake/llvm у меня)
  -DClang_DIR=путь до clang(Например, /Users/shpkkk/compiler-project/tttc_practice/build_rel/lib/cmake/clang у меня)
  -DCMAKE_BUILD_TYPE=Release ..
```

Для компиляции самого плагина необходимо написать ```ninja``` в директории с **build.ninja**.
Во втором же случае необходимо сначала создать папку с названием ```UnusedVarDetector``` в папке **clang/tools** с бинарниками билда и закинуть туда все файлы из **internal clang plugin**. Затем заходим в **CMakeLists** в **clang/tools** и добавляем следующую строчку: 
```
add_clang_subdirectory(UnusedVarDetector)
```
После этого билдим llvm и в папке с билдом пишем:
```
ninja UnusedVarDetector
```
Для присоединения плагина к компиляции пишем в обоих случаях(только в первом нужно ещё сначала закинуть в папку **lib** сам плагин):
```
./clang -cc1 -fno-rtti \
    -load ../lib/UnusedVarDetector.<расширение динамических библиотек вашей OC> \
    -plugin mark-unused \
    файл.cpp
``` 


