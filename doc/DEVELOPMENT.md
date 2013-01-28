# Development Guidelines

## Introduction
This document describes guidelines for the contribution and development of QtDropbox.
Please read on if you plan to contribute your ideas and/or code to the project as many things will be clarified in here.

> This document contains guidelines! So please treat it like this.
> Guidelines are no fixed rules, so if you see need to break them do so - but only do so if absolutely necessary not just because
> they seem to be inconvenient.

## Project Philosophy
### Mission
> Our mission is to create a library for C++/Qt to provide easy access to the services of [Dropbox](http://www.dropbox.com/).

### Linking External Libraries
A great part about providing *easy* access is to keep the projects dependencies as little as possible. There are many
really cool libraries out there that would provide great enhancement to the features of the features. Whenever you
think that a certain library would make things easier for us keep one thing in mind: *Just because it is easy for us
it will be more complex for the users of QtDropbox as they have to satisfy another dependency.*

So how to add third party libraries? If you add them make sure the user of QtDropbox has to activate the library with a
specific switch. So the user can always choose to disable a specific third party library and just go by using the plain
old features provided by QtDropbox itself. As you can guess this requires you to *extend* and *not replace* the feature
set provided by QtDropbox.

#### Example
You found a great library that could be used to enhance the capabilities of QDropboxJson (used to interpret JSON objects 
returned by Dropbox). Of course you can use this library th extend QDropboxJson but make sure the user has a possibility
to exclude the third party library from the project: The user has to compile QtDropbox with a specific switch (e.g.
the user has to add `DEFINES += USE_EXT_JSONLIB` to `qtdropbox.pro`).

Now you can extend QDropboxJson to use the external library when the switch is defined. Else the standard QDropboxJson
is used.

### Adding Qt Modules
As with external libraries try to link as few Qt modules as possible. Whenever you add new functionality that requires
an other Qt module than Network or XML provide a possibility to exclude these modules. If a feature that is provided
by including a new module is so great that we won't miss it, we'll consider adding it to the fixed dependencies of the
library.

### Communication
I usually think writing about a polite and friendly tone is not necessary but... Please be polite and friendly in your
communication - if you are not we feel free to ignore and/or delete your requests. Furthermore keep in mind that we
are working on this project in our spare time and that we are all working in different time zones. Communication can
be slow but your requests will be answered. We apologise for any inconvenience caused thereby.

### E-Mail
You can reach the maintainer of the project by mailing to qtdropbox (at) deder (dot) at. If you want to join the team
use this possibility. If you want to file a bug report you can do this to by mail but filing an issue via github.com is
preferred.

Beware that the project mailing address is only looked after by the principal maintainer of the project. It may take
some time to process your mails. In case of problems it may be faster to open a new issue because there's a higher
possibility that somebody might read it.

### Bug Reports
Whenever you want to file a bug report and hence broken or missing functionality please open a new issue at github. If
you want to request a feature please do so by opening an issue too.

### Patches
You can contribute your code in two ways:
  1. Create a Pull Request on github
  1. Send your patch to the project mailing address: qtdropbox (at) deder (dot) at
We will look into your changes and decide to either accept or refuse them. You will be notified in both cases. Please
keep in mind that the project mailing address is only read by the principal maintainer and may take more time (see above).

## Coding Conventions
The coding conventions of QtDropbox are not very strict. There are only a few requirements to the code:

1. Indentation has to be 4 characters wide.
1. Add meaningful debug output to functions.
1. Keep nested statements simple. (rule of thumb: not more than 2 nested statements)
1. Private member start with _ (underscore)
1. Add doxygen comments to public items. (also use \todo, \warning and \bug gotchas!)

For all other issues use your built-in common sense or refer to a [C++ Coding Standard](http://www.possibility.com/Cpp/CppCodingStandard.html).

The main goal of every code is to work and to be maintainable - so please keep it readable!

## Unit Testing
Whenever you change any functionality you should execute all unit test cases as a form of regression testing. Additionally
you should create new unit tests for not yet covered test cases (e.g. newly implemented functions) and fixed user problems.
