# vecs Code Style Guidelines

## 1. **Naming Conventions**

### 1.1 Classes
- **Format**: PascalCase
- **Example**: `MyClass`, `EntityManager`, `Vector3`

### 1.2 Functions and Methods
- **Format**: snake_case
- **Example**: `get_position()`, `set_position()`, `update_system()`

### 1.3 Variables
- **Format**: snake_case
- **Example**: `entities_count`, `velocity_vector`, `max_speed`

### 1.4 Constants
- **Format**: UPPERCASE_WITH_UNDERSCORES
- **Example**: `MAX_BUFFER_SIZE`, `PI`, `DEFAULT_COLOR`

### 1.5 Data Types
- **Format**: PascalCase
- **Example**: `Vector3`, `Matrix4x4`, `Color`

## 2. **Indentation and Spacing**

- **Spaces**: Use **4 spaces** per indentation level (no tabs).
- **Braces**: Braces should be placed on the same line as the control statement (not on a new line).
  ```cpp
    if (condition) {
        // Code
    }
  ```

- **Wrapping**: Wrap content like this when exceeding `80` characters in one line.
  ```cpp
  void function(
    Param1 param1, 
    Param2 param2, 
    Param3 param3,
    Param4 param4,
    Param5 param5,
  ) {
    // do work with params...
  }
  ```

## 3. ** Others **
- **Encapsulation**: order members from most "public" to most "private". (top to bottom)
  ```cpp
    public:
      int public_variable;
      void public_function();

    protected:
      int _protected_variable;
      void _protected_function();

    private:
      int _underscore_prefix;
      void _private_function();
  ```

- **Comments**: start comments with uppercase letter and end with a final punctuation.
  ```
  // This is a comment.

  /*
    This is a multi-line
    comment.
  */
  ```