# Gx

Gx is a bash (and zsh) solution that allows you to bookmark often used commands and e**x**ecute them later.

## Why would I want to use something like this?

One common approach to simplify the re-use of commands is to create aliases. An even better approach is to put said commands in executable files as this gives you more flexibility in terms of argument handling and more.

Not every small thing makes sense to store in a file though and it can get tricky to remember all aliases once they grow in number.

Also if you have worked with aliases in the past then you might have found the limitation where any additional arguments always have to come at the end. Wouldn't it be nice if you could specify where the arguments are placed in your aliased command?

The gx script lets you store (bookmark) often used commands and even plain bash code so that you can execute them at will later. You can then simply do a `gx -l` to list the stored bookmarks should you need a reminder for what you bookmarked a rarely used command as.

This may not be revolutionary, but if you are already addicted to the [go](https://github.com/bashmarklets/go-west) script then you definitely owe it to yourself to give gx a go.

## Installation

Download or clone this repository and add the following to your `~/.bashrc` file:
```bash
source path/to/gx.inc
```

## Example usage
```
$ gx --help                                                                    
Usage: gx [OPTION?] [BOOKMARK?]

Bookmark often used commands and execute them later on the fly.

  -a, --add                      adds a command with the given key
  -l, --list                     lists current bookmarks and commands
  -r, --remove                   removes a given bookmark
      --clear                    removes all bookmarks
      --purge                    removes temporary bookmarks, pinned bookmarks
                                 are not affected by purge
      --pin                      pin a bookmark
      --unpin                    removes the pin from a bookmark
      --temp                     mark a bookmark as temporary
      --untemp                   unmark a bookmark as temporary
      --description              adds a description to be shown when listing
                                 the bookmark rather than the actual command
      --lookup                   show the command stored for a given bookmark
  -h, --help                     display this help section
  -k, --keys                     lists current keys
      --locate                   list location of data file

Examples:
  gx -a "ls -l ~" lshome         bookmarks the command as "lshome"
  gx -l                          lists currently stored commands
  gx lshome                      executes the command stored as "lshome"
  gx -r lshome                   removes the bookmark with the key "lshome"

```

Example output:
```
[bash@marklet:~]
$ gx execute
 ___________________________________
/ Did you know that 32% of dentists \
\ recommend brushing your teeth?    /
 -----------------------------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||

[bash@marklet:~]
```

## How are arguments handled?

By default additional arguments are added at the end of the command just like they are for normal aliases.

For example if you have a bookmark "dir" for the command of "ls" then you can pass arguments to it.
```
[bash@marklet:~]
$ gx -a "ls" dir                                                                                  
ls added as dir

[bash@marklet:~]
$ gx dir
Desktop                              Music                       Trash
Documents                            Pictures                    Templates
Downloads                            Public                  

[bash@marklet:~]
$ gx dir Desktop/
love_letter.xls                      Mom                         notes.doc
app.desktop                          passwords.txt               recipe.txt
```

You can, however, specify where arguments are placed by using regular bash syntax (`$1`, `$2`, `$@`, etc.).
```
[bash@marklet:~]
$ gx -a 'find $1 -name "*.txt"' txt
find $1 -name "*.txt" added as txt

[bash@marklet:~]
$ gx txt Desktop
Desktop/passwords.txt
Desktop/recipe.txt

```

In this context, as we only specify the use of the very first argument, any additional ones are ignored.

Note the use of single quotes which prevents bash from substituting variables. If you use double quotes then you'll have to make sure to escape things properly.

Keep in mind that it is still bash so if you'd want to then you could add logic to check if an argument has been provided or not, e.g.
```
[bash@marklet:~]
$ gx -a 'if [[ $# = 0 ]]; then echo "Please provide a directory" else find $1 -name "*.txt"; fi' txt
if [[ $# = 0 ]]; then echo "Please provide a directory" else find $1 -name "*.txt"; fi added as txt

[bash@marklet:~]
$ gx txt                                                                                          
Please provide a directory else find -name *.txt
```

This should allow you to set up slightly more complex aliases than before.

## Sorry, but I still prefer using aliases

Old habits are hard to kick, one can understand that. Fret not because you can still use aliases while taking advantage of the features of the gx script.

The `gx --setup_aliases` command sets up aliases for all your bookmarks, e.g. for the bookmark "lshome" an alias of `alias lshome="go lshome"` will be added. Add this call to your .bashrc and you are good to go.
