# cite JSON Schema

This project uses a JSON format based on BibJSON. This documentation describes the expected structure.

## Overview

- You can provide a single record as a JSON object, a list of records (array), or a "collection" object with a `records` array.
- Keys are lowercase, no spaces, and usually singular.
- Authors, editors, identifiers, licenses, and links are lists of objects.
- Journals are objects.
- Simple strings remain simple; more complex fields are represented as objects.

## Example Collection

```json
{
  "metadata": {
    "collection": "my_collection",
    "label": "My collection of records",
    "description": "A comprehensive test collection.",
    "id": "long_complex_uuid",
    "owner": "testuser",
    "created": "2021-01-01T10:00:00.000Z",
    "modified": "2025-05-29T16:46:41.000Z",
    "source": "http://example.com/collection.bib",
    "records": 3,
    "from": 0,
    "size": 3
  },
  "records": [
    {
      "collection": "my_collection",
      "type": "article",
      "title": "Open Bibliography for Science, Technology and Medicine",
      "author": [
        {"name": "Richard Jones"},
        {"name": "Mark MacGillivray"}
      ],
      "year": "2011",
      "journal": {
        "name": "Journal of Cheminformatics",
        "shortcode": "J. Cheminf.",
        "id": "jcheminf",
        "identifier": [
          {
            "id": "1758-2946",
            "type": "issn"
          }
        ],
        "volume": "3",
        "pages": "47"
      },
      "link": [
        {
          "url": "http://www.jcheminf.com/content/3/1/47",
          "anchor": "Full Text"
        }
      ],
      "identifier": [
        {
          "type": "doi",
          "id": "10.1186/1758-2946-3-47",
          "url": "http://dx.doi.org/10.1186/1758-2946-3-47"
        }
      ],
      "license": [
        {
          "type": "copyheart",
          "url": "http://copyheart.org/manifesto/",
          "description": "A great license",
          "jurisdiction": "universal"
        }
      ],
      "id": "rec1"
    },
    {
      "collection": "my_collection",
      "type": "book",
      "title": "A Great Book",
      "author": [
        {
          "name": "Erdös, Paul",
          "alternate": ["Paul Erdos"],
          "firstname": "Paul",
          "lastname": "Erdös",
          "id": "paulerdos"
        }
      ],
      "editor": [
        {
          "name": "MacGillivray, Mark",
          "firstname": "Mark",
          "lastname": "MacGillivray"
        }
      ],
      "year": "1939",
      "publisher": "Famous Publisher",
      "identifier": [
        {
          "type": "isbn",
          "id": "978-3-16-148410-0"
        }
      ],
      "license": [
        {
          "type": "publicdomain",
          "url": "https://creativecommons.org/publicdomain/zero/1.0/"
        }
      ],
      "id": "rec2",
      "url": "http://example.com/my_collection/rec2"
    }
  ]
}
```

## Field Reference

- **collection**: Arbitrary string identifying the collection/group.
- **type**: `article`, `book`, etc.
- **title**: Title of the work.
- **author**: List of objects with at least a `name` field. May also have `firstname`, `lastname`, `alternate`, `id`.
- **editor**: List of objects with at least a `name` field (optional).
- **year**: Publication year.
- **journal**: Object with fields such as `name`, `shortcode`, `id`, `volume`, `pages`, and an `identifier` list.
- **publisher**: String (for books, etc.)
- **link**: List of objects with at least `url`, may have `anchor`.
- **identifier**: List of objects with at least `type` and `id`, may have `url`.
- **license**: List of objects with `type`, `url`, and possibly `description`, `jurisdiction`.
- **id**: Unique string for the record.
- **url**: URL related to the record.

You can add additional fields as needed for your use-case.

## Notes

- This schema is based on BibJSON, but is not identical.
- Not all features or fields of the original BibJSON are supported or required.
- For best compatibility, keep your keys lowercase and use lists/objects as shown above.
