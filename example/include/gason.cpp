#include "gason.hpp"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
///////////////////////////////////////////////////////////////////////////////
namespace gason {
///////////////////////////////////////////////////////////////////////////////
#define JSON_ZONE_SIZE      4096
#define JSON_STACK_SIZE     32

struct JsonAllocator::Zone {
protected:
    Zone*   prev;
    Zone*   next;
    size_t  used;

    friend class JsonAllocator;

public:
    static Zone*    create(size_t allocSize) {
        assert(allocSize <= JSON_ZONE_SIZE);
        Zone* z     = (Zone*) ::malloc(JSON_ZONE_SIZE);
        assert( z != 0 );
        z->used     = allocSize;
        z->next     = nullptr;
        z->prev     = nullptr;

        return z;
    }

protected:
    static Zone*    begin(Zone* head) {
        Zone* iter = head;
        while ( iter ) {
            Zone* prev = iter->prev;
            if ( prev == nullptr )
                break;
            iter       = prev;
        }

        return iter;
    }

    static Zone*    end(Zone* head) {
        Zone* iter = head;
        while ( iter ) {
            Zone* next = iter->next;
            if ( next == nullptr )
                break;
            iter       = next;
        }

        return iter;
    }

    static size_t   totalAllocatedSpace(Zone* head) {
        size_t total = 0;
        Zone* iter   = begin(head);

        while ( iter ) {
            total     += JSON_ZONE_SIZE;
            Zone* next = iter->next;
            iter       = next;
        }

        return total;
    }
};

void*
JsonAllocator::allocate(size_t size) {
    size = (size + 7) & ~7; // ensure the size = 8n and 8n >= size

    if ( head ) { // head is a valid pre-allocated
        assert( head->used <= JSON_ZONE_SIZE );

        if ( head->used + size <= JSON_ZONE_SIZE ) { // head has enough empty spaces
            char *p     = (char*)head + head->used;
            head->used += size;
            return p;
        }

        else if ( head->next ) { // head if full, go for next valid Zone
            Zone* zone  = head->next;
            assert(zone->used == sizeof(Zone));
            assert(zone->prev == head);
            zone->used += size;
            head        = zone;
            return (char*)zone + sizeof(Zone);
        }
    }


    size_t allocSize = sizeof(Zone) + size;
    Zone* zone       = Zone::create(allocSize);

    zone->prev = head;
#ifdef GASON_DEBUG_ALLOCATOR
    fprintf(stderr, "allocating %p (prev: %p , next: %p)\n", zone, zone->prev, zone->next);
#endif
    if ( head )
        head->next = zone;
    head = zone;

    return (char*)zone + sizeof(Zone);
}

void
JsonAllocator::deallocate() {
#ifdef GASON_DEBUG_ALLOCATOR
    size_t totalUsedSpace = Zone::totalAllocatedSpace(head);
    fprintf(stderr, "\n%s(%d): %lu bytes are going to be deallocated.\n",
           __FILE__, __LINE__,
           totalUsedSpace);
#endif

    // goto last allocated item
    Zone* iter = Zone::end(head);

    while ( iter ) {
#ifdef GASON_DEBUG_ALLOCATOR
        fprintf(stderr, "freeing    %p (prev: %p , next: %p)\n", iter, iter->prev, iter->next);
#endif
        Zone *prev = iter->prev;
        free(iter);
        iter = prev;
    }

    head  = nullptr;
}

void
JsonAllocator::reset() {
    Zone* iter = Zone::end(head);

    while ( iter ) {
        memset((char*)iter + sizeof(Zone), 0, JSON_ZONE_SIZE - sizeof(Zone));
        iter->used  = sizeof(Zone);
        Zone* prev  = iter->prev;
        if ( prev == nullptr ) // break if head is the first allocated Zone.
            break;
        iter        = prev;
    }

    head = iter;
}

static inline bool
isdelim(char c) {
    return isspace(c) || c == ',' || c == ':' || c == ']' || c == '}' || c == '\0';
}

static inline int
char2int(char c) {
    if (c >= 'a')
        return c - 'a' + 10;
    if (c >= 'A')
        return c - 'A' + 10;
    return c - '0';
}

static double
string2double(char *s, char **endptr) {
    char ch = *s;
    if (ch == '+' || ch == '-')
        ++s;

    double result = 0;
    while (isdigit(*s))
        result = (result * 10) + (*s++ - '0');

    if (*s == '.') {
        ++s;

        double fraction = 1;
        while (isdigit(*s)) {
            fraction *= 0.1;
            result += (*s++ - '0') * fraction;
        }
    }

    if (*s == 'e' || *s == 'E') {
        ++s;

        double base = 10;
        if (*s == '+')
            ++s;
        else if (*s == '-') {
            ++s;
            base = 0.1;
        }

        int exponent = 0;
        while (isdigit(*s))
            exponent = (exponent * 10) + (*s++ - '0');

        double power = 1;
        for (; exponent; exponent >>= 1, base *= base)
            if (exponent & 1)
                power *= base;

        result *= power;
    }

    *endptr = s;
    return ch == '-' ? -result : result;
}

static inline JsonNode*
insertAfter(JsonNode *tail, JsonNode *node) {
    if (!tail)
        return node->next = node;
    node->next = tail->next;
    tail->next = node;
    return node;
}

static inline JsonValue
listToValue(JsonTag tag, JsonNode *tail) {
    if (tail) {
        JsonNode* head = tail->next;
        tail->next     = nullptr;
        return JsonValue(tag, head);
    }
    return JsonValue(tag, nullptr);
}

JsonParseStatus
jsonParse(char *s, char **endptr, JsonValue *value, JsonAllocator &allocator) {
    JsonNode *tails[JSON_STACK_SIZE];
    JsonTag tags[JSON_STACK_SIZE];
    char *keys[JSON_STACK_SIZE];
    int pos = -1;

    // reset allocator Zones.
    allocator.reset();

    bool separator = true;

    *endptr = s;

    while (*s) {
        JsonValue o;

        while (*s && isspace(*s))
            ++s;

        *endptr = s++;
        switch (**endptr) {
        case '\0':
            continue;
        case '-':
            if (!isdigit(*s) && *s != '.') {
                *endptr = s;
                return JSON_PARSE_BAD_NUMBER;
            }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            o = JsonValue(string2double(*endptr, &s));
            if (!isdelim(*s)) {
                *endptr = s;
                return JSON_PARSE_BAD_NUMBER;
            }
            break;
        case '"':
            o = JsonValue(JSON_TAG_STRING, s);
            for (char *it = s; *s; ++it, ++s) {
                int c = *it = *s;
                if (c == '\\') {
                    c = *++s;
                    switch (c) {
                    case '\\':
                    case '"':
                    case '/':
                        *it = c;
                        break;
                    case 'b':
                        *it = '\b';
                        break;
                    case 'f':
                        *it = '\f';
                        break;
                    case 'n':
                        *it = '\n';
                        break;
                    case 'r':
                        *it = '\r';
                        break;
                    case 't':
                        *it = '\t';
                        break;
                    case 'u':
                        c = 0;
                        for (int i = 0; i < 4; ++i) {
                            if (!isxdigit(*++s)) {
                                *endptr = s;
                                return JSON_PARSE_BAD_STRING;
                            }
                            c = c * 16 + char2int(*s);
                        }
                        if (c < 0x80) {
                            *it = c;
                        } else if (c < 0x800) {
                            *it++ = 0xC0 | (c >> 6);
                            *it = 0x80 | (c & 0x3F);
                        } else {
                            *it++ = 0xE0 | (c >> 12);
                            *it++ = 0x80 | ((c >> 6) & 0x3F);
                            *it = 0x80 | (c & 0x3F);
                        }
                        break;
                    default:
                        *endptr = s;
                        return JSON_PARSE_BAD_STRING;
                    }
                } else if (iscntrl(c)) {
                    *endptr = s;
                    return JSON_PARSE_BAD_STRING;
                } else if (c == '"') {
                    *it = 0;
                    ++s;
                    break;
                }
            }
            if (!isdelim(*s)) {
                *endptr = s;
                return JSON_PARSE_BAD_STRING;
            }
            break;
        case 't':
            for (const char *it = "rue"; *it; ++it, ++s) {
                if (*it != *s)
                    return JSON_PARSE_BAD_IDENTIFIER;
            }
            if (!isdelim(*s))
                return JSON_PARSE_BAD_IDENTIFIER;
            o = JsonValue(JSON_TAG_BOOL, (void *)true);
            break;
        case 'f':
            for (const char *it = "alse"; *it; ++it, ++s) {
                if (*it != *s)
                    return JSON_PARSE_BAD_IDENTIFIER;
            }
            if (!isdelim(*s))
                return JSON_PARSE_BAD_IDENTIFIER;
            o = JsonValue(JSON_TAG_BOOL, (void *)false);
            break;
        case 'n':
            for (const char *it = "ull"; *it; ++it, ++s) {
                if (*it != *s)
                    return JSON_PARSE_BAD_IDENTIFIER;
            }
            if (!isdelim(*s))
                return JSON_PARSE_BAD_IDENTIFIER;
            break;
        case ']':
            if (pos == -1)
                return JSON_PARSE_STACK_UNDERFLOW;
            if (tags[pos] != JSON_TAG_ARRAY)
                return JSON_PARSE_MISMATCH_BRACKET;
            o = listToValue(JSON_TAG_ARRAY, tails[pos--]);
            break;
        case '}':
            if (pos == -1)
                return JSON_PARSE_STACK_UNDERFLOW;
            if (tags[pos] != JSON_TAG_OBJECT)
                return JSON_PARSE_MISMATCH_BRACKET;
            if (keys[pos] != nullptr)
                return JSON_PARSE_UNEXPECTED_CHARACTER;
            o = listToValue(JSON_TAG_OBJECT, tails[pos--]);
            break;
        case '[':
            if (++pos == JSON_STACK_SIZE)
                return JSON_PARSE_STACK_OVERFLOW;
            tails[pos] = nullptr;
            tags[pos] = JSON_TAG_ARRAY;
            keys[pos] = nullptr;
            separator = true;
            continue;
        case '{':
            if (++pos == JSON_STACK_SIZE)
                return JSON_PARSE_STACK_OVERFLOW;
            tails[pos] = nullptr;
            tags[pos] = JSON_TAG_OBJECT;
            keys[pos] = nullptr;
            separator = true;
            continue;
        case ':':
            if (separator || keys[pos] == nullptr)
                return JSON_PARSE_UNEXPECTED_CHARACTER;
            separator = true;
            continue;
        case ',':
            if (separator || keys[pos] != nullptr)
                return JSON_PARSE_UNEXPECTED_CHARACTER;
            separator = true;
            continue;
        default:
            return JSON_PARSE_UNEXPECTED_CHARACTER;
        }

        separator = false;

        if (pos == -1) {
            *endptr = s;
            *value = o;
            return JSON_PARSE_OK;
        }

        if (tags[pos] == JSON_TAG_OBJECT) {
            if (!keys[pos]) {
                if (o.getTag() != JSON_TAG_STRING)
                    return JSON_PARSE_UNQUOTED_KEY;
                keys[pos] = o.toString();
                continue;
            }
            tails[pos] = insertAfter(tails[pos], (JsonNode *)allocator.allocate(sizeof(JsonNode)));
            tails[pos]->key = keys[pos];
            keys[pos] = nullptr;
        } else {
            tails[pos] = insertAfter(tails[pos], (JsonNode *)allocator.allocate(sizeof(JsonNode) - sizeof(char *)));
        }
        tails[pos]->value = o;
    }
    return JSON_PARSE_BREAKING_BAD;
}

JsonValue
JsonValue::child(const char* key) const {
    for ( JsonIterator it = begin(*this);    it != end(*this);    it++) {
        if ( strncmp(it->key, key, strlen(key)) == 0 )
            return it->value;
    }

    return JsonValue();
}

JsonValue
JsonValue::at(size_t index) const {
    if ( getTag() == JSON_TAG_ARRAY ) {
        size_t i = 0;
        for ( JsonIterator it = begin(*this);    it != end(*this);    it++) {
            if ( i++ == index )
                return it->value;
        }
    }

    return JsonValue();
}
///////////////////////////////////////////////////////////////////////////////
} // namespace gason
///////////////////////////////////////////////////////////////////////////////
