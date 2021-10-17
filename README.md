## Simple YAML Parser

A simple YAML parser which produces a Node Tree Object representation of
YAML Documents and includes a find method to locate individual Nodes
within the parsed Node Tree.

```bash
$ git clone https://github.com/trulede/simple_yaml.git
$ cd simple_yaml
$ make
./simple_yaml
Document 0
  kind = Pod
  name = static-web
Document 1
  kind = Service
  name = my-service
  app = MyApp
  targetPort = 9376
```

## Credits

#### Andrew Sydney Poelstra

Helpful learning material and examples:
https://www.wpsoftware.net/andrew/
https://www.wpsoftware.net/andrew/pages/libyaml.html

#### Tyler Barrus

Useful C Libraries. No point reinventing the wheel.
https://github.com/barrusthttps://github.com/barrust
