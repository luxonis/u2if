git submodule update --init --recursive

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

rm -rf ${DIR}/firmware/source/build 
mkdir ${DIR}/firmware/source/build
cd ${DIR}/firmware/source/build

rm ./u2if_CUSTOM_1V8.uf2
cmake .. -DBOARD=CUSTOM_1V8 -DIS_1V8=1
make -j$(nproc --all)
cp ./u2if_CUSTOM_1V8.uf2 ${DIR}/dist/u2if_1v8.uf2

rm ./u2if_CUSTOM_3V3.uf2
cmake .. -DBOARD=CUSTOM_3V3 -DIS_1V8=0
make -j$(nproc --all)
cp ./u2if_CUSTOM_3V3.uf2 ${DIR}/dist/u2if_3v3.uf2

rm ./u2if_SL6945_1V8.uf2
cmake .. -DBOARD=SL6945_1V8 -DIS_1V8=1
make -j$(nproc --all)
cp ./u2if_SL6945_1V8.uf2 ${DIR}/dist

