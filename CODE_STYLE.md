# vecs Code Style Guidelines

---
## 1. **Naming Conventions**

### 1.1 Data Types
- **Format**: type_t
- **Example**: `vec3_t`, `mat4x4_t`, `color_t`

### 1.2 Functions and Methods
- **Format**: snake_case
- **Identation**: function name should always be the first string on the line.
- **Example**: `position()`, `update_system()`
  ```cpp
  constexpr void
  function_name() {

  }
  ```

### 1.3 Variables
- **Format**: snake_case
- **Example**: `entities_count`, `velocity_vector`, `max_speed`

### 1.4 Constants
- **Format**: UPPERCASE_WITH_UNDERSCORES
- **Example**: `MAX_BUFFER_SIZE`, `PI`, `DEFAULT_COLOR`

### 1.5 Encapsulation
- **Format**: order members from most "public" to most "private". (top to bottom)
- **Example**:
  ```cpp
  struct example_t {
  public:
      struct public_struct_t {};
      using public_alias_t = public_struct_t;
      int public_number { 1 };
      void public_function();

  private:
      struct _private_struct_t {};
      using _private_alias = private_struct;
      int _private_member;
      void _private_function();
  }
  ```

### 1.6 Templates
- **Format for typenames**: PascalCase
- **Example**:
  ```cpp
  // Put a space between "template" and "<>"
  template <typename Component>
  void
  process_component() {}

  // Use `T` to refer to a generic Type.
  template <typename T>
  using generic_array_t = std::array<T>;
  ```

### 1.7 Concepts
- **Format**: PascalCase
- **Example**:
  ```cpp
  template <typename T>
  concept Integral = std::is_integral_v<T>;
  ```

### 1.8 Initialization
- **Desc**: Prefer Uniform Initilization `{}`.
- **Example**:
  ```cpp
  int value{};        // Yes.
  int value2 { 10 };  // Yes.

  int value;          // No.
  int value = 10;     // No.
  ```

### 1.9 Const
- **Format**: East const.
- **Examples**: `int const value`, `type_t const& type_ref`
---

## 2. **Indentation and Spacing**

### 2.1 Spaces and Braces
- **Spaces**: Use **4 spaces** per indentation level (no tabs).
- **Braces**: Braces should be placed on the same line as the control statement (not on a new line).
  ```cpp
    if (condition) {
        // Code
    }
  ```

### 2.2 Horizontal character limit
- **Wrapping**: Wrap content when exceeding `80` characters in one line (<u>comments can exceed this limit</u>).
- **Examples**:
  ```cpp
  std::unordered_map<std::type_info*,                                           |
                     std::array<type_t<T...>, sizeof...(T)>> _interesting_map{};|
                                                                                |
  assert(really_long_number_variable < really_long_number_threshold             |
         && "Really long number variable is less than threshold");              |
                                                                                |
  void                                                                          |
  function(                                                                     |   80 character line.
    LongParam1 const long_param1,                                               | 
    LongParam2 const long_param2,                                               | 
    LongParam3 const long_param3,                                               |
    LongParam4 const long_param4,                                               |
    LongParam5 const long_param5,                                               |
  ) const noexcept {                                                            |
    // do work with your long params...                                         |
    // really loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong comments are fine.
  }                                                                             |
  ```
  - **Note**: If you're using VSCode you can set a visual ruler adding the following settings to your `settings.json`:
  ```json
  "editor.rulers": [
      80
  ],
  ```

---
## 3. **Others**
- **Comments**: Comments start with uppercase letter and end with a final punctuation.
  ```
  // This is a comment.

  /*
    This is a multi-line
    comment.
  */
  ```