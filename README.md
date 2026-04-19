# Why?
Well, because i could. And because i had to, kind of.

# What is this exactly?

It defines a generic structure for numbers and a generic API for their operation (like `generic_add()`). Your can define your own number type and provide the needed interface, then the end user can use the generic API to use your custom number type.

Using this (unsurprisingly not for production) library, you can construct different types of numbers (rational, complex, ...) and do arithmetic between them, if supported. When a number is composed of two or more numbers, the types of those numbers are also under your control. For example you can construct the following numbers:

1. Rational number with complex type as numerator and denominator
```
2 + 3j
--------
3 + 1j
```

2. Complex number with rational number as either of real or imaginary type
```
 1
--- + 3j
 2
```

3. Rational number with rational type as numerator

```
    1
   ---
    2
---------
    3
```

I was to lazy to complete the generic API for some cool things to happen, but if you're interested here's what can be done:

- **1.Not crash when operation is not supported**: Yes i was too lazy to do it, yet i put it here.
- **2.Arithmetic between distinct types**: If type A can be casted to type B, `A->ops->to[B]` is not `NULL`. The generic API can use this to do arithmetic between two different type.
- **3.Convert two numbers indirectly**: The `ops->to` array of different types can be combined to form a matrix, which can be used to find a path between two types and convert one to another. This can enhance the second feature listed here.
- **4.Alternative implementation of operations**: In case the type doesn't implement a certain feature, the generic API can use other implemented operations to make the un-supported, supported.
