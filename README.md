# Sapphire

A second try, using clap-first tech, and bringing
the Sapphire dsp to your daw

To build

```bash
git clone https://github.com/baconpaul/sapphire-plugins
cd sapphire-plugins
git submodule update --init --recursive
cmake -Bignore/bld -DCMAKE_BUILD_TYPE=Release
cmake --build ignore/bld --target sapphire-plugins_all
```

Right now has

- elastika with a panel
- tube unit with params but no panel