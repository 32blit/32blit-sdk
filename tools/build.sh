pip3 install --upgrade 32blit
rm -r ../build.stm32/
mkdir ../build.stm32
cd ../build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../32blit.toolchain
make
mkdir  blits
cp examples/*/*.blit blits/
