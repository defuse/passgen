passgen
=======

`passgen` is a unix command line tool for generating passwords. It generates
passwords by turning bytes from `/dev/urandom` into characters in the desired
character set using the try-and-throw-away algorithm from Section 9.7 of
Cryptography Engineering.

It supports generating passwords with characters chosen from printable ASCII,
alphanumeric, and hex characters, as well as a string of random words.

Examples
--------

    $ passgen --ascii
    |;aeQ;_mLh$7!eLVtm3MUm|068El?<Va>)i=IK#+uv&{gN+}18bNp:BCuq[^,"B~

    $ passgen --alpha
    g8168URzTJYuxhEjI3LTsoC9tRfwuwhZz4GtRFiJbPh3ZuhNvJs8qaHvb4OxdHL6

    $ passgen --hex
    4BE6BDA72CB2BB6CC5E163B6209B7489733031C4D150DC013AA4477D26C863C8

    $ passgen --words
    bryan.dress.zap.mist.eben.ahead.debar.gospel.nylon.event

    $ passgen --hex -p 5
    753924DC422047A0D9FFDDEE87BCF6BA65D992EE317178D1C77BDE46DAC13C42
    1ABFFDA08CD24BBD34590D183EE25C610A6B9CCD9847081A786B0061EF312769
    2C065D5BD06412C6BE08C47F728D8AB9A099B5C42102517897426D9CF5420DCA
    239EDCE8E3788F8E86383411EBA7A3E819F8897C263327AA20503D563E59733B
    C2A980F8DFCC686F389B5CB96D30701C22D0B7B6BF2D732F7CD1364D81D949CC

Audit Status
------------

`passgen` has not yet been subjected to a professional security audit.
