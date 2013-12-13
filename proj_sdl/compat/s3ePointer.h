/*
 * (C) 2001-2012 Marmalade. All Rights Reserved.
 *
 * This document is protected by copyright, and contains information
 * proprietary to Marmalade.
 *
 * This file consists of source code released by Marmalade under
 * the terms of the accompanying End User License Agreement (EULA).
 * Please do not use this program/source code before you have read the
 * EULA and have agreed to be bound by its terms.
 */
#ifndef S3E_POINTER_H
#define S3E_POINTER_H

#include "s3eTypes.h"

/**
 * @addtogroup s3egroup
 * @{
 */

/**
 * @defgroup pointerapigroup S3E Pointer API Reference
 *
 * For more information on the Pointer functionality provided by the S3E module,
 * see the "S3E Pointer Overview" section of the <em>S3E API Documentation</em>.
 *
 * @{
 */

/**
 * Describes the type of pointer used by the device.
 * @see s3ePointerProperty
 * @par Required Header Files:
 * s3ePointer.h
 */
typedef enum s3ePointerType
{
    S3E_POINTER_TYPE_INVALID            = 0,
    S3E_POINTER_TYPE_MOUSE              = 1,
    S3E_POINTER_TYPE_STYLUS             = 2
} s3ePointerType;

/**
 * Describes the type of stylus.
 * @see s3ePointerProperty
 * @par Required Header Files:
 * s3ePointer.h
 */
typedef enum s3eStylusType
{
    S3E_STYLUS_TYPE_INVALID             = 0,
    S3E_STYLUS_TYPE_STYLUS              = 1,
    S3E_STYLUS_TYPE_FINGER              = 2
} s3eStylusType;

/**
 * The maximum number of simultaneous touches that can be reported by this API
 */
#define S3E_POINTER_TOUCH_MAX 10

/**
 * Pointer Properties.
 * @see s3ePointerGetInt
 * @par Required Header Files:
 * s3ePointer.h
 */
typedef enum s3ePointerProperty
{
    /**
     * [read,int] Pointer is available on device
     */
    S3E_POINTER_AVAILABLE               = 0,

    /**
     * [read,write,int]
     * Setting this to 1 will disable the cursor on system
     * that have an OS/hardware cursor.  Default is 0.
     */
    S3E_POINTER_HIDE_CURSOR             = 1,

    /**
     * [read, int] Returns the type of pointer, one of @ref s3ePointerType.
     */
    S3E_POINTER_TYPE                    = 2,

    /**
     * [read, int] If the pointer is a stylus, returns the type of stylus, one of @ref s3eStylusType.
     */
    S3E_POINTER_STYLUS_TYPE             = 3,

    /**
     * [read, int] Returns true if multitouch is available.
     */
    S3E_POINTER_MULTI_TOUCH_AVAILABLE   = 4

} s3ePointerProperty;

/**
 * Retrieves an integer property for this subdevice.
 * @param property property to get.
 * @return
 * - The value of the requested property if @e property is valid.
 * - -1 otherwise.\n
 *         Call s3ePointerGetError() to retrieve an error code.
 *         This will return:
 *   - @ref S3E_POINTER_ERR_PARAM if @e property is not valid.
 * @par Required Header Files:
 * s3ePointer.h
 */
S3E_API int32 s3ePointerGetInt(s3ePointerProperty property);

/**
 * Sets an integer property for this subdevice.
 * @param property property to set.
 * @param value New value for property.
 * @see s3ePointerGetInt
 * @return
 * - @ref S3E_RESULT_SUCCESS if no error occurred.
 * - @ref S3E_RESULT_ERROR if the operation failed.\n
 *         Call s3ePointerGetError() to retrieve an error code.
 *         This will return:
 *   - @ref S3E_FILE_ERR_PARAM if @e property is invalid or @e value is invalid.
 *
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API s3eResult s3ePointerSetInt(s3ePointerProperty property, int32 value);

/**
 * Pointer Callbacks.
 * The following callback types can be registered to enable user notification of pointer events.
 * @see s3ePointerRegister, s3ePointerUnRegister
 * @par Required Header Files
 * s3ePointer.h
 */
typedef enum s3ePointerCallback
{
    /**
     * This allows you to create callback is called whenever a pointer button event occurs.
     *
     * Any callback created to respond to this event should conform to the following:
     * @param systemData is a pointer to an @ref s3ePointerEvent struct.
     * @return No return value is expected from a callback registered using this enumeration.
     */
    S3E_POINTER_BUTTON_EVENT,

    /**
     * This event is triggered each time the underlying OS generates a pointer motions event.
     *
     * Any callback created to respond to this event should conform to the following:
     * @param systemData is a pointer to an @ref s3ePointerMotionEvent struct.
     * @return No return value is expected from a callback registered using this enumeration.
     */
    S3E_POINTER_MOTION_EVENT,

    /**
     * Similar to @ref S3E_POINTER_BUTTON_EVENT but only generated on multi-touch devices.
     * Even on a multi-touch device regular @ref S3E_POINTER_BUTTON_EVENT events are still
     * generated for the first touch.
     *
     * Any callback created to respond to this event should conform to the following:
     * @param systemData is a pointer to an @ref s3ePointerTouchEvent struct.
     * @return No return value is expected from a callback registered using this enumeration.
     */
    S3E_POINTER_TOUCH_EVENT,

    /**
     * Similar to @ref S3E_POINTER_MOTION_EVENT but only generated on multi-touch devices.
     * Even on a multi-touch device regular @ref S3E_POINTER_MOTION_EVENT events are still
     * generated for the first touch.
     *
     * Any callback created to respond to this event should conform to the following:
     * @param systemData is a pointer to an @ref s3ePointerTouchMotionEvent struct.
     * @return No return value is expected from a callback registered using this enumeration.
     */
    S3E_POINTER_TOUCH_MOTION_EVENT,
    S3E_POINTER_CALLBACK_MAX
} s3ePointerCallback;

/**
 * Register a callback for a given event.
 *
 * The available callback types are listed in @ref s3ePointerCallback.
 * @param cbid ID of the callback to register.
 * @param fn Callback function.
 * @param userData Value to pass to the @e userData parameter of @e fn.
 * @return
 * - @ref S3E_RESULT_SUCCESS if no error occurred.
 * - @ref S3E_RESULT_ERROR if the operation failed.\n
 *           Call @ref s3ePointerGetError() to retrieve an error code.
 *           This will return one of:
 *   - @ref S3E_POINTER_ERR_PARAM if @e cbid is not a valid member of @ref s3ePointerCallback or @e fn is NULL.
 *   - @ref S3E_POINTER_ERR_TOO_MANY If the maximum number of callbacks for this device has been exceeded.
 *     - @ref S3E_POINTER_ERR_ALREADY_REG If (@e cbid, @e fn) has already been registered.
 * @see s3ePointerUnRegister
 * @note It is not necessary to define a return value for any registered callback.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API s3eResult s3ePointerRegister(s3ePointerCallback cbid, s3eCallback fn, void* userData);

/**
 * Unregister a callback for a given event.
 * @param cbid ID of the callback to unregister.
 * @param fn Callback function.
 * @return
 * - @ref S3E_RESULT_SUCCESS if no error occurred.
 * - @ref S3E_RESULT_ERROR if the operation failed.\n
 *           Call @ref s3ePointerGetError() to retrieve an error code.
 *           This will return one of:
 *   - @ref S3E_POINTER_ERR_PARAM if @e cbid is not a valid member of @ref s3ePointerCallback, or @e fn is NULL.
 *     - @ref S3E_POINTER_ERR_NOT_FOUND if @e fn is not registered for callback @e cbid.
 * @see s3ePointerRegister
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API s3eResult s3ePointerUnRegister(s3ePointerCallback cbid, s3eCallback fn);

/**
 * Pointer Errors.
 * @see s3ePointerGetError
 * @see s3ePointerGetErrorString
 * @par Required Header Files
 * s3ePointer.h
 */
typedef enum s3ePointerError
{
    S3E_POINTER_ERR_NONE        = 0, ///< No Error.
    S3E_POINTER_ERR_PARAM       = 1, ///< Invalid parameter.
    S3E_POINTER_ERR_TOO_MANY    = 2, ///< Maximum number of callbacks exceeded.
    S3E_POINTER_ERR_ALREADY_REG = 3, ///< The specified callback is already registered.
    S3E_POINTER_ERR_NOT_FOUND   = 4, ///< The specified callback was not found for removal.
    S3E_POINTER_ERR_UNAVAIL     = 5  ///< POINTER is unavailable.
} s3ePointerError;

/**
 * Retrieves the last error, if any, for this subdevice.
 * @see s3ePointerGetErrorString
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API s3ePointerError s3ePointerGetError();

/**
 * Retrieves the last error as a string, if any, for this subdevice. This is only available in debug builds.
 * @see s3ePointerGetError
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API const char* s3ePointerGetErrorString();

/**
 * All possible pointer button states.
 * @par Required Header Files
 * s3ePointer.h
 */
typedef enum s3ePointerState
{
    S3E_POINTER_STATE_UP       = 0, ///< Pointer is up.
    S3E_POINTER_STATE_DOWN     = 1, ///< Pointer is down.
    S3E_POINTER_STATE_PRESSED  = 2, ///< Pointer was pressed between the last 2 calls to update states.
    S3E_POINTER_STATE_RELEASED = 4, ///< Pointer was released between the last 2 calls to update states.
    S3E_POINTER_STATE_UNKNOWN       ///< Pointer's status is unknown , keep as the last item.
} s3ePointerState;

/**
 * Pointer buttons that can be queried via s3ePointerGetState.
 * @par Required Header Files
 * s3ePointer.h
 */
typedef enum s3ePointerButton
{
    S3E_POINTER_BUTTON_SELECT          = 0, ///< Tap on a touchpad, same as left mouse click.
    S3E_POINTER_BUTTON_LEFTMOUSE       = 0, ///< Left mouse click, same as tap on touchpad.
    S3E_POINTER_BUTTON_RIGHTMOUSE      = 1, ///< Right mouse click.
    S3E_POINTER_BUTTON_MIDDLEMOUSE     = 2, ///< Middle mouse click.
    S3E_POINTER_BUTTON_MOUSEWHEELUP    = 3, ///< Mouse wheel up.
    S3E_POINTER_BUTTON_MOUSEWHEELDOWN  = 4, ///< Mouse wheel down.
    S3E_POINTER_BUTTON_MAX                  ///< Max number of buttons, keep as the last item
} s3ePointerButton;

/**
 * Structure passed to the @ref S3E_POINTER_BUTTON_EVENT callback as @e systemData.
 */
typedef struct s3ePointerEvent
{
    /** Button that was pressed or released. */
    s3ePointerButton m_Button;
    /** Whether the button was pressed (1) or released (0). */
    uint32           m_Pressed;
    /** Position X. */
    int32            m_x;
    /** Position Y. */
    int32            m_y;
} s3ePointerEvent;

/**
 * Structure passed to the @ref S3E_POINTER_MOTION_EVENT callback as @e systemData.
 */
typedef struct s3ePointerMotionEvent
{
    /** Position X. */
    int32 m_x;
    /** Position Y. */
    int32 m_y;
} s3ePointerMotionEvent;

/**
 * Structure passed to the @ref S3E_POINTER_TOUCH_EVENT callback as @e systemData.
 */
typedef struct s3ePointerTouchEvent
{
    /**
     * ID of the touch. The ID given to a touch is equal to the number
     * of simultaneous touches active at the time the touch began. This ID
     * can be between 0 and S3E_POINTER_TOUCH_MAX-1 inclusive
     */
    uint32 m_TouchID;
    /**  Whether the touch started (1) or ended (0).*/
    uint32 m_Pressed;
    /** Position X. */
    int32  m_x;
    /** Position Y. */
    int32  m_y;
} s3ePointerTouchEvent;

/**
 * Structure passed to the @ref S3E_POINTER_TOUCH_MOTION_EVENT callback as @e systemData.
 */
typedef struct s3ePointerTouchMotionEvent
{
    /**
     * ID of the touch. The ID given to a touch is equal to the number
     * of simultaneous touches active at the time the touch began. This ID
     * can be between 0 and S3E_POINTER_TOUCH_MAX-1 inclusive
     */
    uint32 m_TouchID;
    /** Position X. */
    int32  m_x;
    /** Position Y. */
    int32  m_y;
} s3ePointerTouchMotionEvent;

/**
 * Gets the current pointer state.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API s3ePointerState s3ePointerGetState(s3ePointerButton button);

/**
 * Gets the current x position.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API int32 s3ePointerGetX();

/**
 * Gets the current y position.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API int32 s3ePointerGetY();

/**
 * Gets the current pointer state for a given touchID.
 * Calling this with touchID 0 is the equivalent of calling
 * s3ePointerGetState(S3E_POINTER_BUTTON_SELECT), even if multitouch is
 * not supported.
 * @param touchID touch ID, must be between 0 and S3E_POINTER_TOUCH_MAX.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API s3ePointerState s3ePointerGetTouchState(uint32 touchID);

/**
 * Gets the current x position for the specified touchID
 * Calling this with touchID 0 is the equivalent of calling s3ePointerGetX(),
 * even if multitouch is not supported.
 * If the touchID is not currently active, this returns the last position
 * recorded when it was active, or (0,0) if it has never been pressed.
 * @param touchID touch ID, must be between 0 and S3E_POINTER_TOUCH_MAX.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API int32 s3ePointerGetTouchX(uint32 touchID);

/**
 * Gets the current y position for the specified touchID
 * Calling this with touchID 0 is the equivalent of calling s3ePointerGetY(),
 * even if multitouch is not supported.
 * If the touchID is not currently active, this returns the last position
 * recorded when it was active, or (0,0) if it has never been pressed.
 * @param touchID touch ID, must be between 0 and S3E_POINTER_TOUCH_MAX.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API int32 s3ePointerGetTouchY(uint32 touchID);


/**
 * Updates the pointer state and latches it.
 * @par Required Header Files
 * s3ePointer.h
 */
S3E_API s3eResult s3ePointerUpdate();

/** @} */
/** @} */

#endif /* !S3E_POINTER_H */
