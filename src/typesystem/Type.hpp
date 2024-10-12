#ifndef TYPE_HPP
#define TYPE_HPP

class Type {
      public:
        enum Atomic { INT, CHR, FLT, VOID, COMPOSITE };

        Atomic getType() const { return type; }
        Type(Atomic type): type(type) {};
        virtual ~Type() {}

      private:
        Atomic type;
};

#endif
