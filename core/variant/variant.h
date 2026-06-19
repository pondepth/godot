*** Begin Patch
*** Update File: core/variant/variant.h
@@
-    template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
-    _FORCE_INLINE_ operator T() const { return static_cast<T>(operator int64_t()); }
+    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
+    _FORCE_INLINE_ operator T() const { return static_cast<T>(operator int64_t()); }
@@
-    template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
-    _FORCE_INLINE_ Variant(T p_enum) :
-            Variant(static_cast<int64_t>(p_enum)) {}
+    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
+    _FORCE_INLINE_ Variant(T p_enum) :
+            Variant(static_cast<int64_t>(p_enum)) {}
*** End Patch
