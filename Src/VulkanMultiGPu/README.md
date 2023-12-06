# Processo de compilacao do VMGPU

## Alteracoes:
- adicionado requisito de vulkan api 1.3 para a RTX de geração >= 3000
- Em "\bp\external\shaderc\libshaderc_util\CMakeLists.txt, adicionado "add_compile_definitions(ENABLE_HLSL)" para resolver erro de compilação;
- Em "\bp\external\shaderc\cmake\setup_build.cmake", removida linha 48 "#find_program(\${PROG_UC}_EXE ${PROGRAM} REQUIRED)" para resolver erro de compilação
- Cmakes da bibliteca bp atualizados para receberam a biblioteca QT6

## Windows:
1. Extrair todas as dependencias da pasta dependencias.
2. Installar o QT6
3. Instalar o vulkan sdk
4. Buildar boost com opcao de complete build e toolset mvsc adequado (verificar manual do boost incluso)(bootstrap.bat, b2.exe --build-dir=build-directory toolset=msvc --build-type=complete stage)
5. Copiar a pasta "libs" gerada para a raiz root da pasta "boost"
6. Se o visual studio acusar falta de bibliotecas, remova o path das bibliotecas boost que o cmake ja havia incluido, e adicione as bibliotecas requisitadas manualmente
7. Copie as dlls Qt6Core e Qt6Gui para a pasta que contem o executavel.
8. Copie a pasta "plugins" da instalacao do qt para a pasta que contem o executavel.
9. adicionar o path completo do obj a ser renderizado
10. Materiais utilizam o formato MTL for PBR https://benhouston3d.com/blog/extended-wavefront-obj-mtl-for-pbr/
11. Texturas referenciadas pelo material precisam ser referenciadas de forma local ex: Textura.png. O blender exporta os mtls com paths completos, ex: C:/Users/User/Documents/Textura.png, causando erros.