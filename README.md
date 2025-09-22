# A1A generator (a1a_gen)

---
## Description

a simple morse code raw-format PCM data generator alike [wrigjl/morseplayer](https://github.com/wrigjl/morseplayer), with Japanese character (和文, wa-bun) support

## Usage

### Example

```
$ a1a_gen
usage: a1a_gen -i [infile] -o [outfile] -d [dot_msec] -p [paris_wpm]
$
```
Note: using both `-d` and `-p` option is prohibited.

#### OpenBSD (with aucat)

```
$ echo "<bt>Hello, world<ar>" | ./a1a_gen -d 50 -i - -o - | aucat -e s16le -i -
```

#### Linux (with pacat)

```
$ echo "<ﾎﾚ>こんにちは、セカイ<ﾗﾀ>" | ./a1a_gen -p 20 -i - -o - | pacat --format=s16le --rate=48000
```

#### Linux (with aplay)

```
$ echo "<ﾎﾚ>こんにちは、セカイ<ﾗﾀ>" | ./a1a_gen -p 20 -i - -o - | aplay -f dat
```

## Options

<dl>
 <dt><code>-i &lt;input file&gt;</code>
 <dd>The name of input file, <code>-</code> for standard input.
 <dt><code>-o &lt;output file&gt;</code>
 <dd>The name of output file, <code>-</code> for standard output. Output data is raw PCM, 48000Hz, signed-16bit (little-endian), stereo format as default.
 <dt><code>-d &lt;speed (dot length, msec)&gt;</code>
 <dd>Morse code speed by dot length, default <code>-d 60</code>.
 <dt><code>-p &lt;speed (PARIS WPM)&gt;</code>
 <dd>Morse code speed by PARIS WPM.
</dl>

## License

MIT License
