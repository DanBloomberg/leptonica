/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*!
 * \file  bootnumgen1.c
 * <pre>
 *
 *   Function for generating prog/recog/digits/bootnum1.pa from an
 *   encoded, gzipped and serialized string.
 *
 *   This was generated using the stringcode utility, slightly edited,
 *   and then merged into a single file.
 *
 *   The code and encoded strings were made using the stringcode utility:
 *
 *       L_STRCODE  *strc;
 *       strc = strcodeCreate(101);   // arbitrary integer
 *       strcodeGenerate(strc, "recog/digits/bootnum1.pa", "PIXA");
 *       strcodeFinalize(\&strc, ".");
 *
 *   The two output files, autogen.101.c and autogen.101.h, were
 *   then slightly edited and merged into this file.
 *
 *   Call this way:
 *       PIXA  *pixa = l_bootnum_gen1();   (C)
 *       Pixa  *pixa = l_bootnum_gen1();   (C++)
 * </pre>
 */

#include <string.h>
#include "allheaders.h"

/*---------------------------------------------------------------------*/
/*                         Serialized string                           */
/*---------------------------------------------------------------------*/
static const char *l_bootnum1 =
    "eJy9nAdUU9kX7u8lkFACCQgYakJTRMBLC0GQhCpiA2w4lglVVFSwICqahBIQUcAKikJAHRwb"
    "dsWWEJoKAo4FFJUgOhbUIIpBA3mhXGbeXOe/bnxrnotI1l1Z+v1yztn7O/sU9aCliWGUuVGr"
    "1yxdtZLiqD5j3YrwqNWUVdGUuKWJlEkUBwdndXXvVf/ymfBViVFr5J+C1Ac+vgBaNJGSuHro"
    "iS1lw/A79a1BMyZrqhuqAwCgOSXAN0T+myh/kUD5X8CTJwye/JdaXMD8NQD852D4GMeBh2v9"
    "Qtf6rFqxImrlWgA63X39d/lD2hRfr9mq+RESWaGIQBcSEriEdANTz8hECVsEETczY7DFOOZn"
    "0ISj6xFJU52dlCPCOAV8JG8ADO9bkKiAp+XAfzDFb4bvKW9m8qBsB9SyNUZkK++ZeAOVbIfm"
    "z9s95A+tYNlf2aIUJSFGhYu1/AxkeVJ4uFBtjKoJk5koBYAFJ0d/+nXhBiOERkfUGjXlr9GD"
    "GiVPJbaoNDp69bVy5Q8nDWukMQVxjGQeJ4WzHUxtNiDlCTJMsj44QdIXTImFQCxMFk1WEaaT"
    "8YSZkPFKZgyTDWpRlIBLOpqbj07weYbQ7oRau9bI99t+resbKu1O1G+dA73JC/5+ZwnFDC6P"
    "k87bnsY7yOFdBzP7z0HSP8MkZkKxF1eEI+Td4UM3OZzrHI5BMolRxEkTAQQhzgjA/TqKOy1x"
    "MQeh31kh/UPf/ai3PhN+Tv8SoXgyl7d9UL9J1t1Lg9pxvo7MnWkiOhFUMSLtcIXOu0JlGG0N"
    "H7qGD5kgJBhwyVTg43q9FbnFFuMQ+l0U6jv6g/qdOOO3otLvXHpowzn5wwmw/ucCcSFHRCb1"
    "hTEloD+ZRDJlNzlguWvw+G4SKZJEbMOFWuFBINVVe46Jg20lQi1VoSAy9G3bbP+0FpVal+f5"
    "U6zkDyfDavl1YiCj2cBU7ORRtiRRWijqZXHTZQamtU4eK6Mlz/liGaBrQuc3JrF5XQDGKSc+"
    "h3czV+ThJKRv4cq+ALoRprqUqesTERSuqCm0R/q8s8H4QlQUVP8c0sAnp8AUNXKhKdc5Iswa"
    "jFCYoolPD5M4EGWrmDG4dmzR/mqhJr6ewJGAQXgunSHuwhGTSNYyUukHH/fGaUWHZDIjYMFc"
    "e6XX0WcPITho/2Xfd3XYZ60mf0iF4w6f38hnA+p4PDmU1N8NSfeFScYKxbiMWNKWZiiJXWTI"
    "xbngcW2vCaqTpBhgXuJob70PSt0I0W4KdaGhDi9qMmtCJZrGs3bJkz90GxKtd/uMJkBRBQSu"
    "+ntP4M/E2TUrXyrT3XlnXsOBDkrywXp7Q0nsfBNr4yi3kLMnJ2LObTmU1EvS62je98UGodtB"
    "sQSqYN93ezhBZaCRPOFeQ2mIYxeKICefNU6Y717YHgxeGvcZqFwh7zdLmJK6gb5PMomModfy"
    "W/He9Z14TSme0Ak+c9crjTlV6I1Ujz6P6spfZoPqu/wuosujcPqfAatv6OeIcC9wWGiahC8W"
    "+4BCARmLT+dcNshSITV7SQh4p7bqoAvvg/APZWCMPpGcFMQXs8FUTg/w/jt4/iuBKCuUgYDd"
    "GKfUcmu9UUga9BlXR/4iD9JUjC+yREUDuwL7kajJYovI30EumdHKTiUDXeSEDOMjEoJTo4d/"
    "0RU8ESiWOUEAcIkGcTcUfruDVIs+x+qOqG1waYxApRb2BwuG1NrdEaixGXjfpgn6zMAWrdhb"
    "WrM9mud2Rr6n1EzKiNe39CBFq65Q/eam8SHNIuvcOug2pfbo3ZcPDwXXlueYdthHiGcInIxW"
    "uJhvt3l0nJz1vub0n7EvMRagvY0dj/YaiYY+/Q60oukg2qsLpJuo0OD0O2cILerOGU02RTdV"
    "VBgaWq5U4pDJrQqwXfiiAAhU10linrC90bH6SsmKYlr8x/ZpHOtZ0oY5SotSXk+Z/d3566uW"
    "J2Ma7CrV79SUi3LGajdKTm4EHG1crOvWkRqRUOhzst7IWDFNf8pGBQXn5MARqyzCCWW4oCQg"
    "04idBybLwC4yhesjd3Bk9jJTpa4XKXTSDkw8ndiNcXCMYbax89YQ1VzZ05hMCVBF6ZHJnIDt"
    "Xxj3uceK3JAk6PP1qJGeZ/fmjiEqEjhfzx0mSeTn8OOAHWTImKkc3ovD18h84tx1XcUYh6BT"
    "YOnRXoaSz3e5td5COSTjcER0XXK/gN+IP5yf6xCk2Q2qYLFFvZpB3WSILjeyMgCY+nzCdzct"
    "bzskFfr8/ROjH87fw1Qr6uWdDiIqC/yU7VPm3tAN9DKPC5iaHtPxhm8GVlyL8k6eMuMXk3VE"
    "12KmJLZAaxOttXa69pWEpaOyzAyLDSZdDL/5acmXGdvEPaPe76l0eqNyrAtSxi3ajUzmDuiz"
    "+UCUoAxSTTSIMkJFBWfzESf7PIgfx+ZweKCKBjv+Ow6D7Seo4D+RPaE6FlBJoENSumpVfVCr"
    "VmUnUUrQlAlSZF2YL7UsAHiEdVvZU3vLHgmAPrP/RGeDM3voEMCkwVhAZDTNjHqj7vp4e5sq"
    "SeNhRdDl6u2MKV73R+lse9TXcI7kQtMd9fJE4NKKB791aF1ytVql9b3lK977Vs7v78rKXyZs"
    "VtcM1y9ev+Gzv0aP0jsVu2WT68KQbtERfeLXGYlxauvXcFBxwYl/5hDXHHl341MGwndwUHTq"
    "iYoYX90T61oqLQ4ftkgPKdKdmRDv6aSlPaX2sVW5744jsQ3JX+vjbSjqwmjpF8nGe2+vTjpA"
    "OFva/9usCaJkm5Rjot3LkTjonYD+SHT7UhadggoHdgLTR9yvNB1UZkH3oSRJmFDclo0XehH7"
    "azDV9cy6Cj6+QmaQUTHwtlG1grEZJ8hLF5kRU+pkQfL5oJCPjVmuKWRV04GWE5BU00wfh4RR"
    "bOo9BGPTb1yACgY2ApZwgAsSNDIFfCZoTg6FkoqwSvvXsFN4HDBVKw4Aqp6az08JpfUjNaJP"
    "/wPpxGJQ46yZSdtQaYTTf9Rw+q9zUE9hkHxFY/Uft6zct7xTlKqqP+76zp3bTrg2r51l2+uk"
    "s42w2AF/Rm1dsfmOnfOXt1stt/hjRUzjgiV3PM4uyB8lWnRfOs+3oOeiV9LYTRk9n1kZ2Qcv"
    "7rI7Q7p03OIeocjaXfOx/odFSEjFjMBQ9Gqu2hKOChI2AsMeJ2ooJvsEl+juPpHTtFLZP8PX"
    "65bqCaXb5qai1yGUZIsi79/mPXO9ZvH4ZT30R1mir2x/a46EsrV8KsV+3Ld4UlIWTT2I/udz"
    "G5/gc8FOpbj3R/lGLi+oqX/WTruJRENvB/RH4todPrAYFRpsB2aP2IESrkiGM6Jg7xHYEiDn"
    "KVCJp+JljFABVzQzWVOYnWrIJTBcyByJGZGMbY+mfGAxhXxCZW2bDDQomk8NWsOTgLopHYah"
    "DBJQ8snlGfRpNBvJhN4YDPRJ80GmvFGn7VExwcbglyGmRfU8eUzT9WuaSlyrXF0aeT6Q+jow"
    "yzuFH/LVD9PnPCej5MNU0/WEjkXx0tTY3aNOmd2vetS0+bNll26LdYPyBWHo0gMzXq79uL/m"
    "InsKLpi1U++Q2QSeP0P5rN3JrUgy9ObgJ8IbbA5CYXPA05Z3RN+mBmK+TkigcpSZOeme/piD"
    "qTzQsv1VhTZeqau1cNHjnUHm3m0F2xMTNHdfNDBjmVQEKi3P9wh1L31XXX269cq+dX6T5hw6"
    "XRu1ILm1cK8jfc7OGblILvT24CdaDLYHwcMtVsvTlE8iVBoLKfyDJ98ZuhruTAXztlLyXlAZ"
    "cxd+G8dLphzaQFsaSgGXbtzKLCkq876e7JkyqUXIH3/hfLrFyTDVEgFrY28j7YLR6DSfkMOd"
    "pU+RQOjtwk+4bNguzP+bixuYOjwP9GfunkcjHamsuADZV6ju1lWe5Jyg/2vXzvXrzx8LKe0I"
    "L5izf8nH0yfdu3znp1cFkPKL6pa0HrJ4cykg5cqdLzWTDgYbjWve9ahfs2frBPzzu3rnEWBO"
    "6P2C/khLrbHUtUMFBvuF+fDYgn1QkWU8qPOojGk9VcOoPbxcbSVjSrxb6gy7xX1B42ennu3s"
    "MLximXN/zdqLV8ieO7zM1ArGRxaRGm81PT7w29K2D2fxude+rLrmnzPfNHqfi8ulQ5N/AIbe"
    "OYweaTHD1ZrXUYEhagg1kmwRwcdESNBw5xJYHSRT6QvMixRmXXWcmG7k84I1hhbtK8DHJKn6"
    "ly7BROQYULpN+3dhnnTx/HeIJnNMGp+x8IzvoLKbY3ofiUxC0qC3DvgRmiqP9cWoaGDrYADT"
    "iOVeG8jU0oU8inSUDFUwwI4CK+0jfPdSpDDFygVD/Wd9UyURlTDYLzCHffTtM5opEElFtMZv"
    "q2ut+jP2HkeGZUZImX5sODXRKq2qcIlVseTweC2rYO1Ndkm75zfUNU9cmWK1YDH9QOHr2Z1T"
    "Zmtfvb6C+Mj6sU/BrsZYM60n1nvnZ918pt2rcWYl1X9l+bY8JB96q/ATfLBVCBviWycfH6kD"
    "EZplTvFYz9CwXzG1zZcRMPXoqH0QL9e2F7s6eeyKs90Gu2LIwdpLNKlRPQvuNPSdc3G0SrZx"
    "yDz4qnBGY621zrKtx4WJ8Z8rH0V/nOF6aa7rtu+/fMZlxHmY5cY0aSEBFTMMCgYA2DAsGDEM"
    "N+WGgUzpD8MIyGwJSCSz27GU6LIwCa4CS8Wz1JeRtiSVhknkszrOMgKmK56yEheeUOSJxepj"
    "VZng2z85eSqztyV7hRMi2dEpUNDTFDogrvD0nVWqqoJE+//iG+b9PbbpAk0WuveaHNVLgica"
    "BgoMx9gzejXMP42KKK56Z+92d3z8YTWryG6xt4F3X8uF/BVjzl5gaE9+o/7W2bP1zle+uU34"
    "gorIsAfZet9yjLbX08srTmzYjcRSzDQo2GKwaVg0jFV3Rk1uGtJEv07LrwCN5T/vMzOW07JW"
    "hlMD8tvFHnq+HV3rsskLVEompwfsCQhe73vWLvbo6Tc7F+nslM0a7aUUWuQQw3qV/Ub/Wnfz"
    "9V8P/+ZY7Ln8i7LdJeqG4kDt35BwihUWLAfheBljNVHBwc5hFjxHkskwwniKp2qlJh5PyuvL"
    "BjAyOojtZWGZUhmBKLNmtxPwMjqjy0f+maqBknBGPi+vDyOkW/cLs/l0RqhpsqwZ8/IdHQC2"
    "dvn/kSng05BI6L3DT1Sz/1FqGJptyKfkM/0OBHsAPg7J2irLrcJzs9ruqajTgidmjl0UXy4c"
    "LWw+NkZ3ZsHMmLjpk8heC7xdzSZfOi7Abi2wVl+//Ybw8dGvqzqrJicYGO3YPJFllxQTHfYA"
    "weX8/8U6jESO5/0jc3ML+dycTBAKZB74etn8ojwOxwDjyn7jT+qv4ce1ETBCHFa1v+YuXyyj"
    "q56jQI2tzxnSNnLjDkPKqbKwROD8V4xFY0I/HQiJpDYkqt2Zi0RDbx4GIsfQLIry9eoiVGiw"
    "efAfQZOliAgvCFwyXg8/EBVTVIQcsg4eJwuAnPtfMGOwIpkxvlNmEFBRH3Sd4sSI5tQl8Sng"
    "RUzk6MYZwA2CTY/bhR1FSAr0pmHgvckgxdX9fCdUFLBpMIQphHzAl04ieUJFbAwz3jEXAwhr"
    "SblfPY+1IpUpVmUYqlK9Ox4jQaUMdg0L4SrD8JDIjmltit699oQfzlrFarxgjAHnnkqmG3hx"
    "cfzFtc4LDxw7/CjeaIlTYIpOWV+C2lMfmdh3tpujwPfd/Sb3R233g6P/xC/ZY75to0BSvfs9"
    "WFJs3xEcocZHsinmGIbYlJdiqlGxwY5h1l9sfEhVmZ+98PB5V7G6qZ2F7+1wgzRt63G2qsTM"
    "XUQPghtJuXqbhqu+6Xza14Dr05bo6Nc5vZ1fRTfq9fyQ6Tx1QtMfiZHrKbI91eT3o6zfX+e+"
    "uYREUmyNQcHmgj3C8Br09EEkPMbr6B1VU7+SM2upa6hdqjZzVM+o+aapLZzsMVdnfGXUbetF"
    "jCQrI+0xlgtl41ZtqIi7IlpYc/LosRUs2e7WyUepWzbROn4QshRzBAqOa9gRwNlFwmcKxGyv"
    "ZBFgvAAylsgHOYVI9q/QrGPUARX6DFuwup5OI+2QiZgSWTaYLLOA3HMatHZHYNu5ZOIXHD9h"
    "DiRlSgXi7D4c8OINBDxqj3uHRELvBgjyl/Eg0t6PzfcUQnIdRophhrMxPSy8lFmp5ZMlI5E6"
    "SSQJSavMQRoWLiH4auJlBGYRFute5BZL2YNPAEqtSHdqDdS3IGUrlucVHCWwiQn5q04tjwAq"
    "TeesQ9qDPX6Zy7DVp4Yp7xtXzJ5uE3n9w7s64tiPR7ElV+I1XxYETsmeUMWZnaLiV352+4Un"
    "Y+lWUWOpk50XXLxJnjGaBZLZVlvBOcqOSCLFVhSGoi3TbJk2KiLYucB71pgMjDxTyD3KXHnn"
    "MeOIZCk4YhIx8wAnz4Qj6/IRkzdYX2TxxZhM+v0SkYyJB9Y+dx3l2Xb1FUK3C/o0/hMLvrA9"
    "GSmxC/s5IowF0UkXMmZCQDjIraYr4espUkImT1pNVgpyweIJRc6hfQQs9hULj++sldCT66Xp"
    "HB5Yegn8HRf5iPyKALx9MCZ/DTVXEwmDPnH/xBIv7EmGl3inDy5/EH2askPKsBGHI/T8vW6H"
    "r53/wr/ompKXcbBl1Xjbg5vrYhOdbUmaJbcfaDT9seRac5L7xHI/k/uAX3V+aH/Jwai0q8tc"
    "Y6ff3HgR97B7/DPWtkAKEgp9Hh8oZRgMQn1akLEZFRTsRoaradOzBsvVlSd8k+3DlfzD7G+n"
    "2vaUQEpWKt/+JOrq2s/4lP/6btqasjKnCHzz/phDr3NJmGBWc83VNbHOG+bcmP4t5P64greT"
    "ntZuwdhts7SUVMzuQwKhT/8D1YyhPU2vppjVoAKCjYkpPFTYAMjlKnExKnhChj8OzyintDXg"
    "ANv60ZtP93g2I9UplsANB9UVSrwaUamDzcnwiq1V3RltNkU+5f9IJAbizjNVHvqC2F/y9zVh"
    "rRbGtGVsu1t/58OE6xPXx526ZfDIOq/TVnm8763V+/9I9/n85IFH8PsT1LFJaRd51GN9PM0b"
    "dizwpWDM0sNrIpELAy7oc/hARBpa2PnlcGsXKirYlgxPhjfX8eSdCFBpstAL8GfOvnG8U/VE"
    "SDLjrHdLkY6XNV15e+KnHe4ZGsEnrzFvEbcFUBe4x16o36m5svXgwc0rRjnXnOzXvspqutlG"
    "vp7zq6Hfoc/Aurnu2ZkB1ANILPQZfWBsWA1icV7MPI4KC7YmU0fKFy/IQhmBIS3SyuB8Bikr"
    "2eFcVpwEl8FilwnpA0sdBI7EYWCpI4G0khnBZDLDGcAYbrdmEtFJLSzvIE4GAtsNg87xuxgS"
    "JIpiO/3GDKKoL53ujQoFzuTDS7fr6q5qM5i6vqLubEap8h7DBYZ6MaK3DGvo1JcMvGBfZ53p"
    "6tFXKiqyZ/hm9r5sLT7z2GZ89YruCbhtNZ/M1ub7B0urNFuL9d6uA647T6KKju9Yi8RBn+H1"
    "R1pmpZnlHlQ4cIaP/lvljDEQivOIWz8rWRryrG1o9Vur9864mKqpdsKE2+adPDp0b6x5AdF3"
    "6oOMs8QMT9Xt7fv9Np0o7o+ybxlNLVCRTZM8EkNbwIiVN9zO0icWPLV2fOvZlBjbaxPeqPkb"
    "zSNrP19qiqRUbGFgiDJNv3AvKsp/1Cv8hA5abApepfHmm+rb1S6tl/ceGGNvqlHh17zQPQGa"
    "GzMl8ZbQIsvcVYc967ShOK5DO/GPCvfUjdz71Owj16urtrx/F621t/hkX9iuZg/Zxg9TBc5r"
    "5pQgkKjoDcFADh2qV4zKP4kuOsOGYLi+FLV90JopTwjMdfH4xZ+ZSlEPnKt+IFFuzkLEHJ35"
    "Y6c+LCzMPch/M/sDtCOsAHOqN/ho8No75n5p9UunbNhw6vqYDqz/quoPt7nXzrTS38nedj2K"
    "vy8Dxq6ze6M8LqATCafYsoCC8QI2CFF/344zUIzJCanAhu2q8+MoZVRVjDEwi8qsrV6rq3va"
    "7d3d11ZrdbYHWAU93No66VLijHVY+0kk1rXeTXTlT1iVWGHLXh9xjCxml1qMi1WxG2/ufUF/"
    "tveBtPK8RaZmV+herr4a85GQ6A2DAQDn17F3ly9HBQkbhpkw5NUBSL+m8VcCbqnGVsQ23A43"
    "tbI4fJiRF+pdzHN78D3nMK2l6vBWztbLow/Zbww8n0XQf0E+HjDh2rLfObROk4Ox7b+XlCcB"
    "a8Kg1MON52KROOjtwsCxiiH/03DrDbrNRrBdsBnZOswGNPF4nA5Jy4OZCPjgIOJsDzDVkB3A"
    "4XiBvluCAOCtxFh7n8P1g0il6K3DT5xbga0DA85GggYxO18EWPhgMLplrpCYzMyh88XkDENS"
    "v6ujdEmkpKZOnF4oSp8kTN/Ixci0Mw1IXhnpJsBdUMdK0pyLPBtCRW8SfqL6CpuE4bqMX93w"
    "MqeT7m4os8RBaUa7TdanLI2FU8ISTbmV7e/63RMtq/3X6gZ1niu5FeW+Mn68hmj6Dr+rnGNt"
    "wR9djoMFU2UHd270j/RvHPt7zLyxfaGUPImSfoPHzgMezWeRbIptL1QwrMFsof/XjNO3yUd3"
    "vrBkg64llmk18WHZbhXvydraN3jFiTu3XT1XbViu93b1hX3j9h8LP3xHu5UWHE/ZNCvBpMO8"
    "23xb/yXe2u9lrq6PKhoObtmdCLzeOKmPcfEacgWOit42DES0oTJswSvWKFRcSAfE2yGS4dy5"
    "soENH7quDAcfEyGdqF8aIYkIqhGXFO0QCXGqW1Qr03mXTTJaOHn5oAHkVZcUlK7yxKyD0SOT"
    "4IB1VXRlDr8KWRakorcMOiPd78TeeYGoUP6xA3RdnZnW4HaIpTY2bkrz3jyggfP2l9h6MR9d"
    "KL7lePgotqu1YXJWplhdmFnmu9DrfJXFpeYxG6inqKXpm8mU8izWktPWOw/Sspj7J0zwfd6r"
    "9MzXb0yXBY2PpFLMIii4DAUboV9hIzRMNZN4/0GPNe1UWupsypUQN7W4lNt6B+ynJvt5uHU7"
    "mJzZX9au6qzzWM1QJT91WcX7HQ0b1TVO59tunT7KR6Oy+vTTFerzMemrMi/Z+/8uu2b37iVo"
    "dJN2j3bXHllTc0VvF35iYgE7oOl/H1dEZYGPcqq94Mhl519fXbRtUl3LdFSq7BbvK7tXuS1a"
    "mdNwekaH1+6U0rsJlwJ36X6YtnD1XmhZ8+L+196ut2y3Ta3GnQxxXRs67uxiJIxi+w2HPPjr"
    "C3PdUcHA3id2eO63Y2C5F+9HNrdSwUSq3h7vSPLL9LHFHohUvj3+hMRzRpflgh4W9QN3Jje4"
    "N3dDaHLPlIVzPleKUp5MZuW/2nR0hd/VVaf8b8zyraa69fg6XXa/XTEumryO0vB13cnVVU91"
    "Dp369fMfZB19N+LbI7lkJCt6l6A/0nCXE2syULHCVihspOHUUhnyYC8LavYtA0WG2qeOjOf7"
    "xnbuMV+I4Rrhv6+Ms0iZ8D2fpxfccjlUbe8T7VFH9vu/8t8388i6lPcf2pdnPcvPiVpUfufu"
    "nqx63xup2PhvrtGbnkyWhrAY+c4QN2fhPSQget8wADi0zc+z88IYVICwDYI3lEtYUoKQXk3H"
    "UyQpPK5ocpqmMJ1OxRP6ayEPqVAYJyarnmPHW+i6yiLYEdGUaOV2f+t61TkaAg88gUOK1K+t"
    "I+i5EN1JlIT+P2UYAFhG/TPg6Z4GJBV6jzGQlodKXM/vHetARQW7ofEw1TyBuCZZxFbxwZA2"
    "AyVk3yJCgKqNtUDslSwCcRiicxwASA5QxuO8C0yQUtHbiZ+oxv1gmScVIqo0eY0m3gMfBxKP"
    "XFZWp7Bfe/9y1tfipnwWSIxeoX0t1rk1RVwYNPaT7YyZ0qf7Ldqcn4TM3xWlaxWoH0W4nui9"
    "+WlJ/HTuaa5NMcHoTclblZUCu9ZUh6UXkWz/6WmFf+whHVrCGtxDOs5JKSvhatDhyCMU9YeQ"
    "Uaq9ijltbOajAinh47T7Abu30jRyp5y4u6V8KtneLWPyd/37BvHL/3iX/Ax72VZgZ0/1+Z27"
    "Qvr4QRa1C7Op2+NFomU8cg3eFb2j+Ilmgx1FwF/jBhTKoi3r8AIZI68PFJKjVfvBCkYv4T2B"
    "5InpiqBsIVT+yUv0JN5xsoH4DAwxpd7b8SvZR0mIK6IRMYBWrstu5vKVH5Ac/+kaA8wxsoor"
    "E2OEMkzAZTDTgCIhE2VeFXRjTyiJJuGInILeyzG0Kms4clNkZEApAjNEr4xT92s8nMAXYyy/"
    "/iG3MVWT3mi7LWtBUqC3DzqAwgdIYFMUNEJRwxGlpw0soRM9AUEWo47FFnWzlPsphMo6Ym82"
    "QQvy6G8DqmUsiNkmZLTiK+vIjGYwq4WzUUIwZcbQk9gionuzB+BHox1Ne2qaieChKeYXhnjG"
    "GRuNRcXzj5WfETtE0Ntz/dPlxccPJy9nXGbE6ynnEXV027fe+nB3tX88vtiix/yi0oPoO3c3"
    "6HV9y5pCXHworcqwktwrMv34YIrtjeX9LbtWpj2wiZ147xjyXBkNvWnQARQ+cwE7IPe/jrby"
    "gQpNZidQhSN9o3uXeajSPfllMjHIlVFatao6B5tM3v8EyayisMgrrE84QIZ3fFC3vRGLlI7e"
    "A/xEY8B+B16sho+LbLO2NuDMmRmYu3NPxtmMc+Ba+5COnHGLX64yuaZjMWpvRrfV/luh7LNR"
    "24xmvb7DI7+hmALz5i1vmvf8Rv1W8xW/X4+YeoDOemaTPfWPlMlIJPRZ/yfGC/IEjDabAvg0"
    "rc0SWZcb6i2vKonTpOmzd8y4mfab/unal/ZNC0+Ynth7cuHJ1Ebyk/TFfoG1pR1erP6ytEQj"
    "phN1Od22e46k4PiE/ms0t7Ax++8icf6W7h2hv4Dk7/+tlYhD5xUCj336C8m0YuAFrmxc5/lP"
    "JNjIwGenJcLnfDHnCkcERmCEAowmPp0MQRCLyWROFr5ntErB0ldg0AoLIuaLtG0VADygjB5X"
    "4FbjhVTuopDygffag8rrQoEAVMr/uW81kcXmccAsY5KpMT+dvYwPADa/6kL5YZgMpDiqQuLw"
    "I+IE/nEEVOJge+IAf61B/DgGkGpIIr1ygKQpFJD4LYIv/o0tmgwK03F4AgEyjgWOhTEBoJyn"
    "vbR7OgNCinZVSPS/3YHxP0TDvsPzr2DTCPhMt8X/mUjq74KkuWEDB6PALFO5qW0Lk/ArxNkc"
    "EZ3owgimY1SdyyDjtrBEJgboZusVFd6+dRhJQFOIgDBCoLllfyUqAjgtTxyOObevqgEUPNAk"
    "NN90PKdK+WsceOZ8jq/U+LTHtRd68xaWLVebRfKelblzVG5h2rnrvaqkBmwryQB4+NkAuQub"
    "5qbw9/+jewz+h3o4Hbv9talKjMnyHNj0hvEm9EMeqbwUFkmrGTLOCUvkC8WFaSKWhk54mKSm"
    "opEvbKRggJg60zBccxZyuucGKST+364F+B/i4dw7XJ/0qxgM94Age2GQo7d/mHPsmbyWSoug"
    "csbq2wU7yneHxJmlJbml5En47o+DBFkX5ndd2k5PjTNsoR+ADA63v8cgK8FuDgp3nyEGqbW1"
    "NiqGf+6zGBoA3QQHKa6YCrVHSHC+qymfa3aIcMQHDklACU7GeN1G5wplpP0yh/PyeRu72aRi"
    "3r0OJ6R2R4U7j96g9sXm7ZaotMPpdiLceeRDlZ0uitMQdmHw75nKkcYeFWK2iqWWLy6U9NiA"
    "0d5HIEnI7NoeHD6dwK4ki+WtziB5OpRxkOsmbk7/ddeHM6vPyPVG6aIUTSHGlIuxxdfj8e9x"
    "jAyTLErnKywXg8cTBjYaQtIlYZIGoTg7XZSNFxKKThly5fPvThYwr430aFuLNXJty02xk39D"
    "u6WOb75yHpU5gDPpXwV6CRtUGkXSYotxKtxqsjq+nlEnTedwTAxkwuTcDq+3B7xIpV8pjQUY"
    "NXXmGq+jLOBxd3ofCMjMrLCfCxv3IvUrdgPPT07+zUduxuADvjISqR+CnCEKAxPaQT6Wgcns"
    "l/8rDl8NPrwkBaxHKvxPN93DSTV8SKFLnYO6cMDer7pXuubd3Udx+5ZfOeej1X78tfXbe78X"
    "VMQ4hdRWpESF0eO3UtXOzGqxto28F/cMf7qnu6f/TNC+jGVRZmd7L12JP9B6nBAcnTtvU2Z3"
    "2bNUEb1PnzcB3O0QZ3NvF7JG6KbYif6hjV7ZL0DFbnEKHulDW9giTDfIxWXhSIyyLg7Gx4l4"
    "iQW0k7XwfYxvdFU6jS/OBvVkS4BbDgwxAYvdYh2NCysiOTUWYIsMc65Qur16pLvo8s5w0J71"
    "C7GrGgmk2Gm9/7crJAbPjEOqvk1GKcEecwP3mW4PJglCDNUjV+tcDuh6LMsVBZBrn2btnivk"
    "L/V61mNN3XaWl80E6YGn1cghheNfT4v8hjM4QzVjXPtyH0mi2B48BYc3nJzhyzCGrpAwOEdn"
    "0iQy0MdJ1xXCOGBUx0IQ+4ySfAatgn1FxuP7WPIBVBK2hRItA3fjfg9LKgIe4hy/gqqO8ja5"
    "eGSszqtJZ9YhSBwgxQ7yGw2izC9vd0SFAqfqacOr8MP1pSbykVoSdPv93uQLdWOy4qIyAwSH"
    "jyRuj9r80Cxab2ui6m7Bis3Kj+49zIruF4y17r8v2tC/7uOkPo+nTxZ93bY4dJ4V4N3ToP0D"
    "GMUu9PnJ6yLgw3g0iozD4XixQawhW94w8nkxnS1JAYmjISkLqMR5QtJVyu3RlOh++cAxSZaJ"
    "fRwaC8g9RCmY+s5MAqrKanawZDIAEB70Os19/sXvBzj/aeUcTuPDtb85dwemmPg0UWHKTtDW"
    "uEpga8t0aq3Uth23WsBen6j/665rm9ROqr1Ny4w7F38mpHxSiOXqt78e49JWF67fu4Nxj0G9"
    "4eC2a8+ZqVPLYvNM+o5OFPQnAi19E67UGjGQwc0BUmz6PDSEfG/ey0TFBif5aSP+lsUWgfLo"
    "hpFHN0MSo5nD6QUxmO+4aXX4yiyGGzEGaMcw8O1KaQb9MnbbFspvJod7OJjZKo3Tilpw7IQk"
    "pnwQ3T5Izop100WelHKA/pbunf7mWpx+7FoGUubQdENtW2DvX0D2FgMvMPEIPeHfwjUdBpKk"
    "iNgqQlCNS9XiJmhxu/F4GScjRUTGCQlqXJw+RTkS49fvRMnxIHXFQ1Ac5BwKJUlBwNNPV3JP"
    "qPLiBwwuCjEQR8aP06eqMFQMcIT2HqnIHhKx3IWsBC6LUdkgxmV0GPQ3O0lLcTyCzOt1hLsw"
    "IsF6opZ/H3SfXwcoa+Ip72UtYa/4jawEQCI0W/jQNMv/BxjU/7op/rlXPVEysERBmgJ5SARx"
    "plwtGZPGFDSOEUCFKSIWZuL+53x+A59fIWjkgyE9L3BAAsdkQeLYOeIfaHdVSPu/3cP3P7TD"
    "8XjknqWGVr44/QxHhPORJxLKlhygkmBI0upfyaRJb3J42hlGEg6p9FQvQMS8bKZD/Hh3XVef"
    "LhGB0UfgRHPqIlfJEf4Y4z4+6saRH+DQ/uum+EHNcuA6Pqkx/pMJvscELzMLUMrsB3NVhGQN"
    "LlkfT7ZhvBYQuBg6yZAFOcsbqh8DFPiRWvVJ7l9+oN9N4RExtCeImL5LGZV+OAT7wEWE4Zns"
    "ZPuF0ZbcEqbd5MXKlUr2s9JCre+9Ob5knqlV6huJ9OnSsORxPiQVjOsvjZhJ098XuFVWLaCf"
    "syVnOT/+BiA5/n5JH0qOH+0Y+h8ccLgdHtlzKgZ3pfgJxh4pqlPN2tkU19F++mVseyxjCmd3"
    "4+qJhwL1/Wtw+417j1fe9OJrnj6T/CDXgpd5MNTitGZ5oCF9fHzED25S+vttfSgwVAG4Qonp"
    "wjSiwoDnJfrD3en1EqZyhT832rm5FlDaDkTcxtplp7PSfyDNUeGeThiUNjo08TEqafCExA4+"
    "IFPGZOYw2zDmWtgN0DecP8uStCUASmpjJjbzG1vlo0D+AwIXPqp99Dvalv8DxU4KKR6ocAzV"
    "JW3i7+BRKYYnqdDIPDtblGIsBD25mFj8+xX499Pxnen810BGRzqvxwef3kIyFTt45GCAixeI"
    "AUcuinh/F/1/ACq4jWg=";

/*---------------------------------------------------------------------*/
/*                      Auto-generated deserializer                    */
/*---------------------------------------------------------------------*/
/*!
 * \brief   l_bootnum_gen1()
 *
 * \return   pixa  of labeled digits
 *
 * <pre>
 * Call this way:
 *      PIXA  *pixa = l_bootnum_gen1();   (C)
 *      Pixa  *pixa = l_bootnum_gen1();   (C++)
 * </pre>
 */
PIXA *
l_bootnum_gen1(void)
{
l_uint8  *data1, *data2;
l_int32   size1;
size_t    size2;
PIXA     *pixa;

        /* Unencode selected string, write to file, and read it */
    data1 = decodeBase64(l_bootnum1, strlen(l_bootnum1), &size1);
    data2 = zlibUncompress(data1, size1, &size2);
    pixa = pixaReadMem(data2, size2);
    lept_free(data1);
    lept_free(data2);
    return pixa;
}
