#include "gtest/gtest.h"
#include <ups/upscaledb.h>



/* A structure for the "customer" database */
typedef struct
{
    uint32_t id;         /* customer id; will be the key of the customer table */
    char name[32];        /* customer name */
    /* ... additional information could follow here */
} customer_t;

/* A structure for the "orders" database */
typedef struct
{
    uint32_t id;         /* order id; will be the key of the order table */
    uint32_t customer_id;/* customer id */
    char assignee[32];    /* assigned to whom? */
    /* ... additional information could follow here */
} order_t;

TEST( env, one_to_many )
{
    const uint32_t MAX_DBS = 3;

    const uint32_t DBNAME_CUSTOMER = 1;
    const uint32_t DBNAME_ORDER    = 2;
    const uint32_t DBNAME_C2O      = 3;   /* C2O: Customer To Order */

    const uint32_t DBIDX_CUSTOMER = 0;
    const uint32_t DBIDX_ORDER    = 1;
    const uint32_t DBIDX_C2O      = 2;

    const uint32_t MAX_CUSTOMERS   = 4;
    const uint32_t MAX_ORDERS      = 8;

    uint32_t i;
    ups_status_t st;               /* status variable */
    ups_db_t *db[MAX_DBS];         /* upscaledb database objects */
    ups_env_t *env;                /* upscaledb environment */
    ups_cursor_t *cursor[MAX_DBS]; /* a cursor for each database */
    ups_key_t key;
    ups_key_t cust_key;
    ups_key_t ord_key;
    ups_key_t c2o_key;
    ups_record_t record;
    ups_record_t cust_record;
    ups_record_t ord_record;
    ups_record_t c2o_record;

    ups_parameter_t params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
        {0, 0}
    };

    ups_parameter_t c2o_params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
        {UPS_PARAM_RECORD_SIZE, sizeof(uint32_t)},
        {0, 0}
    };

    customer_t customers[MAX_CUSTOMERS] =
    {
        { 1, "Alan Antonov Corp." },
        { 2, "Barry Broke Inc." },
        { 3, "Carl Caesar Lat." },
        { 4, "Doris Dove Brd." }
    };

    order_t orders[MAX_ORDERS] =
    {
        { 1, 1, "Joe" },
        { 2, 1, "Tom" },
        { 3, 3, "Joe" },
        { 4, 4, "Tom" },
        { 5, 3, "Ben" },
        { 6, 3, "Ben" },
        { 7, 4, "Chris" },
        { 8, 1, "Ben" }
    };

    /* Now create a new database file for the Environment */
    st = ups_env_create( &env, "test.db", 0, 0664, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    /*
   * Then create the two Databases in this Environment; each Database
   * has a name - the first is our "customer" Database, the second
   * is for the "orders"; the third manages our 1:n relation and
   * therefore needs to enable duplicate keys
   */
    st = ups_env_create_db( env, &db[DBIDX_CUSTOMER], DBNAME_CUSTOMER, 0, params );
    ASSERT_TRUE( st == UPS_SUCCESS );

    st = ups_env_create_db( env, &db[DBIDX_ORDER], DBNAME_ORDER, 0, params );
    ASSERT_TRUE( st == UPS_SUCCESS );

    st = ups_env_create_db( env, &db[DBIDX_C2O], DBNAME_C2O, UPS_ENABLE_DUPLICATE_KEYS, c2o_params );
    ASSERT_TRUE( st == UPS_SUCCESS );

    /* Create a Cursor for each Database */
    for (i = 0; i < MAX_DBS; i++)
    {
        st = ups_cursor_create( &cursor[i], db[i], 0, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS );
    }

    /*
   * Insert the customers in the customer table
   *
   * INSERT INTO customers VALUES (1, "Alan Antonov Corp.");
   * INSERT INTO customers VALUES (2, "Barry Broke Inc.");
   * etc
   */
    for (i = 0; i < MAX_CUSTOMERS; i++)
    {
        key.size = sizeof(uint32_t);
        key.data = &customers[i].id;

        record.size = sizeof(customer_t);
        record.data = &customers[i];

        st = ups_db_insert(db[0], 0, &key, &record, 0);
        ASSERT_TRUE( st == UPS_SUCCESS );
    }

    /*
   * And now the orders in the second Database; contrary to env1,
   * we only store the assignee, not the whole structure
   *
   * INSERT INTO orders VALUES (1, "Joe");
   * INSERT INTO orders VALUES (2, "Tom");
   */
    for (i = 0; i < MAX_ORDERS; i++)
    {
        key.size = sizeof(uint32_t);
        key.data = &orders[i].id;

        record.size = sizeof(orders[i].assignee);
        record.data = orders[i].assignee;

        st = ups_db_insert(db[1], 0, &key, &record, 0);
        ASSERT_TRUE( st == UPS_SUCCESS );
    }

    /*
   * And now the 1:n relationships; the flag UPS_DUPLICATE creates
   * a duplicate key, if the key already exists
   *
   * INSERT INTO c2o VALUES (1, 1);
   * INSERT INTO c2o VALUES (2, 1);
   * etc
   */
    for (i = 0; i < MAX_ORDERS; i++)
    {
        key.size = sizeof(uint32_t);
        key.data = &orders[i].customer_id;

        record.size = sizeof(uint32_t);
        record.data = &orders[i].id;

        st = ups_db_insert( db[2], 0, &key, &record, UPS_DUPLICATE );
        ASSERT_TRUE( st == UPS_SUCCESS );
    }

    /*
   * Now start the query - we want to dump each customer with his
   * orders
   *
   * loop over the customer; for each customer, loop over the 1:n table
   * and pick those orders with the customer id. then load the order
   * and print it
   *
   * the outer loop is similar to:
   *  SELECT * FROM customers WHERE 1;
   */
    while (1)
    {
        customer_t *customer;

        st = ups_cursor_move( cursor[0], &cust_key, &cust_record, UPS_CURSOR_NEXT );
        if (st == UPS_KEY_NOT_FOUND)
            break;
        ASSERT_TRUE( st == UPS_SUCCESS );


        customer = (customer_t *)cust_record.data;

        /* print the customer id and name */
        // printf("customer %d ('%s')\n", customer->id, customer->name);

        /*
        * loop over the 1:n table
        *
        * before we start the loop, we move the cursor to the
        * first duplicate key
        *
        * SELECT * FROM customers, orders, c2o
        *   WHERE c2o.customer_id=customers.id AND
        *    c2o.order_id=orders.id;
        */
        c2o_key.data = &customer->id;
        c2o_key.size = sizeof(uint32_t);
        st = ups_cursor_find( cursor[2], &c2o_key, 0, 0 );
        if (st == UPS_KEY_NOT_FOUND)
            break;
        ASSERT_TRUE( st == UPS_SUCCESS );

        st = ups_cursor_move( cursor[2], 0, &c2o_record, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS );

        while( true )
        {
            uint32_t order_id;

            order_id = *(uint32_t *)c2o_record.data;
            ord_key.data = &order_id;
            ord_key.size = sizeof(uint32_t);

            /*
            * load the order
            * SELECT * FROM orders WHERE id = order_id;
            */
            st = ups_db_find( db[1], 0, &ord_key, &ord_record, 0 );
            ASSERT_TRUE( st == UPS_SUCCESS );

            // printf("  order: %d (assigned to %s)\n", order_id, (char *)ord_record.data);

            /*
            * The flag UPS_ONLY_DUPLICATES restricts the cursor
            * movement to the duplicate list.
            */
            st = ups_cursor_move( cursor[2], &c2o_key, &c2o_record, UPS_CURSOR_NEXT|UPS_ONLY_DUPLICATES );
            if( st == UPS_KEY_NOT_FOUND )
                break;
            ASSERT_TRUE( st == UPS_SUCCESS );

        }
    }

    /*
    * Now close the Environment handle; the flag
    * UPS_AUTO_CLEANUP will automatically close all Databases and
    * Cursors
    */
    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    ASSERT_TRUE( st == UPS_SUCCESS );

}
