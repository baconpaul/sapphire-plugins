# Sapphire

A second try, using clap-first tech, and bringing
the Sapphire dsp to your daw

To build

```bash
git clone https://github.com/baconpaul/sapphire-plugins
cd sapphire-plugins
git submodule update --init --recursive
cmake -GNinja -Bignore/bld -DCMAKE_BUILD_TYPE=Release  # see below
cmake --build ignore/bld --target sapphire-plugins_all
```

The `-GNinja` is because the copy-after-build on linux doesnt
work with multi-config generators yet. If you don't have ninja
installed (and don't want to install it for some reason) you can also
use

```
cmake -Bignore/bld -DCMAKE_BUILD_TYPE=Release -DCOPY_AFTER_BUILD=FALSE
```

and then install the resulting artifact by hand. I will fix this soon I'msure.
Right now has

- elastika with a panel
- tube unit with params but no panel
