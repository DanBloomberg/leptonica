# libz
pushd $SRC/zlib
./configure --static --prefix="$WORK"
make -j$(nproc) all
make install
popd

# libzstd
pushd $SRC/zstd
make -j$(nproc) install PREFIX="$WORK"
popd

# libjbig
pushd "$SRC/jbigkit"
make clean
make -j$(nproc) lib
cp "$SRC"/jbigkit/libjbig/*.a "$WORK/lib/"
cp "$SRC"/jbigkit/libjbig/*.h "$WORK/include/"
popd

# libjpeg-turbo
pushd $SRC/libjpeg-turbo
cmake . -DCMAKE_INSTALL_PREFIX="$WORK" -DENABLE_STATIC:bool=on
make -j$(nproc)
make install
popd

# libpng
pushd $SRC/libpng
cat scripts/pnglibconf.dfa | \
  sed -e "s/option WARNING /option WARNING disabled/" \
> scripts/pnglibconf.dfa.temp
mv scripts/pnglibconf.dfa.temp scripts/pnglibconf.dfa
autoreconf -f -i
./configure \
  --prefix="$WORK" \
  --disable-shared \
  --enable-static \
  LDFLAGS="-L$WORK/lib" \
  CPPFLAGS="-I$WORK/include"
make -j$(nproc)
make install
popd

# libwebp
pushd $SRC/libwebp
export WEBP_CFLAGS="$CFLAGS -DWEBP_MAX_IMAGE_SIZE=838860800" # 800MiB
./autogen.sh
./configure \
  --enable-libwebpdemux \
  --enable-libwebpmux \
  --disable-shared \
  --disable-jpeg \
  --disable-tiff \
  --disable-gif \
  --disable-wic \
  --disable-threading \
  --prefix="$WORK" \
  CFLAGS="$WEBP_CFLAGS"
make clean
make -j$(nproc)
make install
popd

# libtiff
pushd "$SRC/libtiff"
cmake . -DCMAKE_INSTALL_PREFIX="$WORK" -DBUILD_SHARED_LIBS=off
make clean
make -j$(nproc)
make install
popd

# leptonica
export LEPTONICA_LIBS="$WORK/lib/libjbig.a $WORK/lib/libzstd.a $WORK/lib/libwebp.a $WORK/lib/libpng.a"
./autogen.sh
./configure \
  --enable-static \
  --disable-shared \
  --with-libpng \
  --with-zlib \
  --with-jpeg \
  --with-libwebp \
  --with-libtiff \
  --prefix="$WORK" \
  LIBS="$LEPTONICA_LIBS" \
  LDFLAGS="-L$WORK/lib" \
  CPPFLAGS="-I$WORK/include -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION"
make -j$(nproc)
make install

for f in $SRC/leptonica/prog/fuzzing/*_fuzzer.cc; do
  fuzzer=$(basename "$f" _fuzzer.cc)
  $CXX $CXXFLAGS -std=c++11 -I"$WORK/include" \
    $SRC/leptonica/prog/fuzzing/${fuzzer}_fuzzer.cc -o $OUT/${fuzzer}_fuzzer \
    -Isrc/ \
    "$WORK/lib/liblept.a" \
    "$WORK/lib/libtiff.a" \
    "$WORK/lib/libwebp.a" \
    "$WORK/lib/libpng.a" \
    "$WORK/lib/libjpeg.a" \
    "$WORK/lib/libjbig.a" \
    "$WORK/lib/libzstd.a" \
    "$WORK/lib/libz.a" \
    $LIB_FUZZING_ENGINE
done

cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/pix_rotate_shear_fuzzer_seed_corpus.zip
cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/enhance_fuzzer_seed_corpus.zip
cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/colorquant_fuzzer_seed_corpus.zip
cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/dewarp_fuzzer_seed_corpus.zip
cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/pix_orient_fuzzer_seed_corpus.zip
cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/pixconv_fuzzer_seed_corpus.zip
cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/blend_fuzzer_seed_corpus.zip
cp $SRC/leptonica/prog/fuzzing/general_corpus.zip $OUT/pixMirrorDetectDwa_fuzzer_seed_corpus.zip

cp $SRC/leptonica/prog/fuzzing/pixa_recog_fuzzer_seed_corpus.zip $OUT/
cp $SRC/leptonica/prog/fuzzing/barcode_fuzzer_seed_corpus.zip $OUT/
