#include "gtest/gtest.h"
#include <ups/upscaledb.hpp>



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

void run_demo()
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
    upscaledb::env env;       /* upscaledb environment */
    upscaledb::db db[MAX_DBS];  /* upscaledb database objects */
    upscaledb::cursor cursor[MAX_DBS]; /* a cursor for each database */
    upscaledb::key key, cust_key, ord_key, c2o_key;
    upscaledb::record record, cust_record, ord_record, c2o_record;

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

    /* Create a new upscaledb environment */
    env.create("test.db");

    /*
   * Then create the two Databases in this Environment; each Database
   * has a name - the first is our "customer" Database, the second
   * is for the "orders"; the third manages our 1:n relation and
   * therefore needs to enable duplicate keys
   *
   * All database keys are uint32 types.
   */
    ups_parameter_t params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
        {0, 0}
    };

    /*
   * The "mapping" between customers and orders stores uint32 customer IDs
   * as a key and uint32 order IDs as a record
   */
    ups_parameter_t c2o_params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
        {UPS_PARAM_RECORD_SIZE, sizeof(uint32_t)},
        {0, 0}
    };

    db[DBIDX_CUSTOMER] = env.create_db(DBNAME_CUSTOMER, 0, params );
    db[DBIDX_ORDER]  = env.create_db(DBNAME_ORDER, 0, params );
    db[DBIDX_C2O]    = env.create_db(DBNAME_C2O, UPS_ENABLE_DUPLICATE_KEYS, c2o_params );

    /* Create a cursor for each database */
    for (i = 0; i < MAX_DBS; i++)
    {
        cursor[i].create( &db[i] );
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
        key.set_size(sizeof(uint32_t));
        key.set_data(&customers[i].id);

        record.set_size(sizeof(customer_t));
        record.set_data(&customers[i]);

        db[0].insert(&key, &record);
    }

    /*
   * And now the orders in the second database; contrary to env1,
   * we only store the assignee, not the whole structure
   *
   * INSERT INTO orders VALUES (1, "Joe");
   * INSERT INTO orders VALUES (2, "Tom");
   */
    for (i = 0; i < MAX_ORDERS; i++)
    {
        key.set_size(sizeof(uint32_t));
        key.set_data(&orders[i].id);

        record.set_size(sizeof(orders[i].assignee));
        record.set_data(orders[i].assignee);

        db[1].insert(&key, &record);
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
        key.set_size(sizeof(uint32_t));
        key.set_data(&orders[i].customer_id);

        record.set_size(sizeof(uint32_t));
        record.set_data(&orders[i].id);

        db[2].insert(&key, &record, UPS_DUPLICATE);
    }

    /*
   * Now start the query - we want to dump each customer with his
   * orders
   *
   * loop over the customer; for each customer, loop over the 1:n table
   * and pick those orders with the customer id. then load the order
   * and print it
   *
   * the outer loop is similar to
   * SELECT * FROM customers WHERE 1;
   */
    while (1)
    {
        customer_t *customer;

        try
        {
            cursor[0].move_next(&cust_key, &cust_record);
        }
        catch (upscaledb::error &e)
        {
            /* reached end of the database? */
            if (e.get_errno() == UPS_KEY_NOT_FOUND)
                break;
            ASSERT_TRUE( e.get_errno() == UPS_SUCCESS );
        }

        customer = (customer_t *)cust_record.get_data();

        /* print the customer id and name */
        //std::cout << "customer " << customer->id << " ('"
        //          << customer->name << "')" << std::endl;

        /*
        * Loop over the 1:n table
        *
        * before we start the loop, we move the cursor to the
        * first duplicate key
        *
        * SELECT * FROM customers, orders, c2o
        *   WHERE c2o.customer_id=customers.id AND
        *    c2o.order_id=orders.id;
        */
        c2o_key.set_data(&customer->id);
        c2o_key.set_size(sizeof(uint32_t));

        try
        {
            cursor[2].find(&c2o_key);
        }
        catch (upscaledb::error &e)
        {
            if (e.get_errno() == UPS_KEY_NOT_FOUND)
                continue;
           ASSERT_TRUE( e.get_errno() == UPS_SUCCESS );
        }

        /* get the record of this database entry */
        cursor[2].move(0, &c2o_record);

        while( true )
        {
            uint32_t order_id;

            order_id = *(uint32_t *)c2o_record.get_data();
            ord_key.set_data(&order_id);
            ord_key.set_size(sizeof(uint32_t));

            /*
            * Load the order
            * SELECT * FROM orders WHERE id = order_id;
            */
            ord_record = db[1].find(&ord_key);

            //std::cout << "  order: " << order_id << " (assigned to "
             //         << (char *)ord_record.get_data() << ")" << std::endl;

            /*
            * the flag UPS_ONLY_DUPLICATES restricts the cursor
            * movement to the duplicate list.
            */
            try
            {
                cursor[2].move(&c2o_key, &c2o_record, UPS_CURSOR_NEXT | UPS_ONLY_DUPLICATES);
            }
            catch (upscaledb::error &e)
            {
                /* reached end of the database? */
                if (e.get_errno() == UPS_KEY_NOT_FOUND)
                    break;
               ASSERT_TRUE( e.get_errno() == UPS_SUCCESS );
            }

        }
    }
}

TEST( env, cpp_interface )
{
    ASSERT_NO_THROW( run_demo() );
}
